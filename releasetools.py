import common
import struct

class Image:
  def __init__(self, name, data):
    self.name = name
    self.data = data

class BadMagicError(Exception):
    __str__ = "bad magic value"

#
# Motoboot packed image format
#
# #define BOOTLDR_MAGIC "MBOOTV1"
# #define HEADER_SIZE 1024
# #define SECTOR_SIZE 512
# struct packed_images_header {
#         unsigned int num_images;
#         struct {
#                 char name[24];
#                 unsigned int start;  // start offset = HEADER_SIZE + start * SECTOR_SIZE
#                 unsigned int end;    // end offset = HEADER_SIZE + (end + 1) * SECTOR_SIZE - 1
#         } img_info[20];
#         char magic[8];  // set to BOOTLDR_MAGIC
# };
HEADER_SIZE = 1024
SECTOR_SIZE = 512
NUM_MAX_IMAGES = 20
MAGIC = "MBOOTV1\0"
def UnpackMotobootImage(data):
  """ Unpack the data blob as a motoboot image and return the list
  of contained image objects"""
  num_imgs_fmt = "<L"
  num_imgs_size = struct.calcsize(num_imgs_fmt)
  num_imgs, = struct.unpack(num_imgs_fmt, data[:num_imgs_size])

  img_info_format = "<24sLL"
  img_info_size = struct.calcsize(img_info_format)

  imgs = [ struct.unpack(img_info_format, data[num_imgs_size + i*img_info_size:num_imgs_size + (i+1)*img_info_size]) for i in range(num_imgs) ]

  magic_format = "<8s"
  magic_size = struct.calcsize(magic_format)
  magic, = struct.unpack(magic_format, data[num_imgs_size + NUM_MAX_IMAGES*img_info_size:num_imgs_size + NUM_MAX_IMAGES*img_info_size + magic_size])
  if magic != MAGIC:
    raise BadMagicError

  img_objs = []
  for name, start, end in imgs:
    start_offset = HEADER_SIZE + start * SECTOR_SIZE
    end_offset = HEADER_SIZE + (end + 1) * SECTOR_SIZE - 1
    img = Image(trunc_to_null(name), data[start_offset:end_offset+1])
    img_objs.append(img)

  imgs = img_objs

  return imgs


def FindRadio(zipfile):
  try:
    return zipfile.read("RADIO/radio.img")
  except KeyError:
    return None


def FullOTA_InstallEnd(info):
  try:
    bootloader_img = info.input_zip.read("RADIO/bootloader.img")
  except KeyError:
    print "no bootloader.img in target_files; skipping install"
  else:
    WriteBootloader(info, bootloader_img)

  radio_img = FindRadio(info.input_zip)
  if radio_img:
    WriteRadio(info, radio_img)
  else:
    print "no radio.img in target_files; skipping install"

def IncrementalOTA_VerifyEnd(info):
  # HACK: We don't yet have radio image patching working,
  #       so just return
  return
  target_radio_img = FindRadio(info.target_zip)
  source_radio_img = FindRadio(info.source_zip)
  if not target_radio_img or not source_radio_img: return
  if source_radio_img != target_radio_img:
    info.script.CacheFreeSpaceCheck(len(source_radio_img))
    radio_type, radio_device = common.GetTypeAndDevice("/radio", info.info_dict)
    info.script.PatchCheck("%s:%s:%d:%s:%d:%s" % (
        radio_type, radio_device,
        len(source_radio_img), common.sha1(source_radio_img).hexdigest(),
        len(target_radio_img), common.sha1(target_radio_img).hexdigest()))

def IncrementalOTA_InstallEnd(info):
  try:
    target_bootloader_img = info.target_zip.read("RADIO/bootloader.img")
    try:
      source_bootloader_img = info.source_zip.read("RADIO/bootloader.img")
    except KeyError:
      source_bootloader_img = None

    if source_bootloader_img == target_bootloader_img:
      print "bootloader unchanged; skipping"
    else:
      WriteBootloader(info, target_bootloader_img)
  except KeyError:
    print "no bootloader.img in target target_files; skipping install"

  tf = FindRadio(info.target_zip)
  if not tf:
    # failed to read TARGET radio image: don't include any radio in update.
    print "no radio.img in target target_files; skipping install"
  else:
    tf = common.File("radio.img", tf)

    sf = FindRadio(info.source_zip)
    if not sf:
      # failed to read SOURCE radio image: include the whole target
      # radio image.
      WriteRadio(info, tf.data)
    else:
      sf = common.File("radio.img", sf)

      if tf.sha1 == sf.sha1:
        print "radio image unchanged; skipping"
      else:
        WriteRadioPatchset(info, tf, sf)

def WriteRadioPatchset(info, target_imagefile, source_imagefile):
    # TODO: Support patching modem partition images.
    #       For now copy the whole radio.img to the OTA package
    WriteRadio(info, target_imagefile.data)

def WriteRadio(info, radio_img):
  info.script.Print("Writing radio...")

  try:
    imgs = UnpackMotobootImage(radio_img)
  except BadMagicError:
      assert False, "radio.img bad magic value"

  WriteGroupedImages(info, "radio", imgs)

def WriteGroupedImages(info, group_name, images):
  """ Write a group of partition images to the OTA package,
  and add the corresponding flash instructions to the recovery
  script.  Skip any images that do not have a corresponding
  entry in recovery.fstab."""

  for i in images:
    try:
      _, device = common.GetTypeAndDevice("/"+i.name, info.info_dict)
    except KeyError:
      print "skipping flash of %s; not in recovery.fstab" % (i.name,)
      continue
    common.ZipWriteStr(info.output_zip, "%s.%s.img" % (group_name,i.name),i.data)

    info.script.AppendExtra('package_extract_file("%s.%s.img", "%s");' %
                            (group_name, i.name, device))

def WriteBootloader(info, bootloader):
  info.script.Print("Writing bootloader...")

  try:
    imgs = UnpackMotobootImage(bootloader)
  except BadMagicError:
      assert False, "bootloader.img bad magic value"

  common.ZipWriteStr(info.output_zip, "bootloader-flag.txt",
                     "updating-bootloader" + "\0" * 13)
  common.ZipWriteStr(info.output_zip, "bootloader-flag-clear.txt", "\0" * 32)

  _, misc_device = common.GetTypeAndDevice("/misc", info.info_dict)

  info.script.AppendExtra(
      'package_extract_file("bootloader-flag.txt", "%s");' %
      (misc_device,))

  # OTA does not support partition changes, so
  # do not bundle the partition image in the OTA package.
  black_list = [ 'partition' ]
  imgs_to_write = [ k for k in imgs if k.name not in black_list ]

  WriteGroupedImages(info, "bootloader", imgs_to_write)

  info.script.AppendExtra(
      'package_extract_file("bootloader-flag-clear.txt", "%s");' %
      (misc_device,))

def trunc_to_null(s):
  if '\0' in s:
    return s[:s.index('\0')]
  else:
    return s
