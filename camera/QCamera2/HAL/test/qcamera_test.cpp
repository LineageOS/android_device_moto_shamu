/* Copyright (c) 2012-2014, The Linux Foundataion. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <ui/DisplayInfo.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>

#include <system/camera.h>

#include <camera/Camera.h>
#include <camera/ICamera.h>
#include <camera/CameraParameters.h>
#include <media/mediarecorder.h>

#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include <cutils/memory.h>
#include <SkImageDecoder.h>
#include <SkImageEncoder.h>

#include "qcamera_test.h"

#define ERROR(format, ...) printf( \
    "%s[%d] : ERROR: "format"\n", __func__, __LINE__, ##__VA_ARGS__)

namespace qcamera {

using namespace android;

int CameraContext::JpegIdx = 0;
int CameraContext::mPiPIdx = 0;
SkBitmap *CameraContext::skBMtmp[2];
sp<IMemory> CameraContext::PiPPtrTmp[2];

/*===========================================================================
 * FUNCTION   : previewCallback
 *
 * DESCRIPTION: preview callback preview mesages are enabled
 *
 * PARAMETERS :
 *   @mem : preview buffer
 *
 * RETURN     : None
 *==========================================================================*/
void CameraContext::previewCallback(const sp<IMemory>& mem)
{
    printf("PREVIEW Callback 0x%x", ( unsigned int ) mem->pointer());
    uint8_t *ptr = (uint8_t*) mem->pointer();
    printf("PRV_CB: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
           ptr[0],
           ptr[1],
           ptr[2],
           ptr[3],
           ptr[4],
           ptr[5],
           ptr[6],
           ptr[7],
           ptr[8],
           ptr[9]);
}

void CameraContext::useLock()
{
    Mutex::Autolock l(mLock);
    if ( mInUse ) {
        mCond.wait(mLock);
    } else {
        mInUse = true;
    }
}

void CameraContext::signalFinished()
{
    Mutex::Autolock l(mLock);
    mInUse = false;
    mCond.signal();
}

/*===========================================================================
 * FUNCTION   : saveFile
 *
 * DESCRIPTION: helper function for saving buffers on filesystem
 *
 * PARAMETERS :
 *   @mem : buffer to save to filesystem
 *   @path: File path
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::saveFile(const sp<IMemory>& mem, String8 path)
{
    unsigned char *buff = NULL;
    int size;
    int fd = -1;

    if (mem == NULL) {
        return BAD_VALUE;
    }

    fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0655);
    if(fd < 0) {
        printf("Unable to open file %s %s\n", path.string(), strerror(fd));
        return -errno;
    }

    size = mem->size();
    if (size <= 0) {
        printf("IMemory object is of zero size\n");
        close(fd);
        return BAD_VALUE;
    }

    buff = (unsigned char *)mem->pointer();
    if (!buff) {
        printf("Buffer pointer is invalid\n");
        close(fd);
        return BAD_VALUE;
    }

    if ( size != write(fd, buff, size) ) {
        printf("Bad Write error (%d)%s\n",
               errno,
               strerror(errno));
        close(fd);
        return INVALID_OPERATION;
    }

    printf("%s: buffer=%08X, size=%d stored at %s\n",
           __FUNCTION__, (int)buff, size, path.string());

    if (fd >= 0)
        close(fd);

    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : PiPCopyToOneFile
 *
 * DESCRIPTION: Copy the smaller picture to the bigger one
 *
 * PARAMETERS :
 *   @bitmap0 : Decoded image buffer 0
 *   @bitmap1 : Decoded image buffer 1
 *
 * RETURN     : decoded picture in picture in SkBitmap
 *==========================================================================*/
SkBitmap * CameraContext::PiPCopyToOneFile(
    SkBitmap *bitmap0, SkBitmap *bitmap1)
{
    int size0;
    int size1;
    SkBitmap *src;
    SkBitmap *dst;
    unsigned int dstOffset;
    unsigned int srcOffset;

    if (bitmap0 == NULL && bitmap1 == NULL) {
        return NULL;
    }

    size0 = bitmap0->getSize();
    if (size0 <= 0) {
        printf("Decoded image 0 is of zero size\n");
        return NULL;
    }

    size1 = bitmap1->getSize();
        if (size1 <= 0) {
            printf("Decoded image 1 is of zero size\n");
            return NULL;
        }

    if (size0 > size1) {
        dst = bitmap0;
        src = bitmap1;
    } else if (size1 > size0){
        dst = bitmap1;
        src = bitmap0;
    } else {
        printf("Picture size should be with different size!\n");
        return NULL;
    }

    for(int i=0; i<src->height(); i++) {
        dstOffset = i*(dst->width())*mfmtMultiplier;
        srcOffset = i*(src->width())*mfmtMultiplier;
        memcpy(((unsigned char *) dst->getPixels())+dstOffset,
            ((unsigned char *) src->getPixels())+srcOffset,
            src->width()*mfmtMultiplier);
    }

    return dst;
}

/*===========================================================================
 * FUNCTION   : decodeJPEG
 *
 * DESCRIPTION: decode jpeg input buffer.
 *
 * PARAMETERS :
 *   @mem : buffer to decode
 *
 * RETURN     : decoded picture in SkBitmap

 *==========================================================================*/
SkBitmap *CameraContext::decodeJPEG(const sp<IMemory>& mem)
{
    SkBitmap *skBM;
    skBM = new SkBitmap; //Deleted in encodeJPEG (skBMtmp[0] and skBMtmp[1])
    SkBitmap::Config prefConfig = SkBitmap::kARGB_8888_Config;
    const void *buff = NULL;
    int size;

    buff = (const void *)mem->pointer();
    size= mem->size();

    switch(prefConfig) {
        case SkBitmap::kARGB_8888_Config:
        {
            mfmtMultiplier = 4;
        }
            break;

        case SkBitmap::kARGB_4444_Config:
        {
            mfmtMultiplier = 2;
        }
        break;

        case SkBitmap::kRGB_565_Config:
        {
            mfmtMultiplier = 2;
        }
        break;

        case SkBitmap::kIndex8_Config:
        {
            mfmtMultiplier = 4;
        }
        break;

        case SkBitmap::kA8_Config:
        {
            mfmtMultiplier = 4;
        }
        break;

        default:
        {
            mfmtMultiplier = 0;
            printf("Decode format is not correct!\n");
        }
        break;
    }

    if (SkImageDecoder::DecodeMemory(buff, size, skBM, prefConfig,
            SkImageDecoder::kDecodePixels_Mode) == false) {
        printf("%s():%d:: Failed during jpeg decode\n",__FUNCTION__,__LINE__);
        return NULL;
    }

    return skBM;
}

/*===========================================================================
 * FUNCTION   : encodeJPEG
 *
 * DESCRIPTION: encode the decoded input buffer.
 *
 * PARAMETERS :
 *   @stream  : SkWStream
 *   @bitmap  : SkBitmap decoded image to encode
 *   @path    : File path
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code

 *==========================================================================*/
status_t CameraContext::encodeJPEG(SkWStream * stream,
    const SkBitmap *bitmap, String8 path)
{
    int qFactor = 100;
    long len;
    status_t ret;
    unsigned char *buff;
    unsigned char temp;

    skJpegEnc = SkImageEncoder::Create(SkImageEncoder::kJPEG_Type);

    if (skJpegEnc->encodeStream(stream, *bitmap, qFactor) == false) {
        return BAD_VALUE;
    }
    printf("%s: buffer=%08X, size=%d stored at %s\n",
        __FUNCTION__, (int)bitmap->getPixels(),
        bitmap->getSize(), path.string());
    delete skBMtmp[0];
    delete skBMtmp[1];

    FILE *fh = fopen(path.string(), "r+");
    if ( !fh ) {
        printf("Could not open file %s\n", path.string());
        return BAD_VALUE;
    }

    fseek(fh, 0, SEEK_END);
    len = ftell(fh);
    rewind(fh);

    if( !len ) {
        printf("File %s is empty !\n", path.string());
        fclose(fh);
        return BAD_VALUE;
    }

    buff = (unsigned char*)malloc(len);
    if (!buff) {
        printf("Cannot allocate memory for buffer reading!\n");
        return BAD_VALUE;
    }

    ret = fread(buff, 1, len, fh);
    if (ret != len) {
        printf("Reading error\n");
        return BAD_VALUE;
    }

    ret = ReadSectionsFromBuffer(buff, len, READ_ALL);
    if (ret != NO_ERROR) {
        printf("Cannot read sections from buffer\n");
        DiscardData();
        DiscardSections();
        return BAD_VALUE;
    }
    free(buff);
    rewind(fh);

    temp = 0xff;
    ret = fwrite(&temp, sizeof(unsigned char), 1, fh);
    if (ret != 1) {
        printf("Writing error\n");
    }
    temp = 0xd8;
    fwrite(&temp, sizeof(unsigned char), 1, fh);

    for (int i=0; i<mSectionsRead; i++) {
        switch((mSections[i].Type)) {

        case 0x123:
            fwrite(mSections[i].Data, sizeof(unsigned char),
                mSections[i].Size, fh);
            break;

        case 0xe0:
            temp = 0xff;
            fwrite(&temp, sizeof(unsigned char), 1, fh);
            temp = 0xe1;
            fwrite(&temp, sizeof(unsigned char), 1, fh);
            fwrite(mJEXIFSection.Data, sizeof(unsigned char),
                mJEXIFSection.Size, fh);
            break;

        default:
            temp = 0xff;
            fwrite(&temp, sizeof(unsigned char), 1, fh);
            fwrite(&mSections[i].Type, sizeof(unsigned char), 1, fh);
            fwrite(mSections[i].Data, sizeof(unsigned char),
                mSections[i].Size, fh);
            break;
        }
    }
    free(mJEXIFSection.Data);
    DiscardData();
    DiscardSections();
    fclose(fh);

    ret = NO_ERROR;

    return ret;
}

/*===========================================================================
 * FUNCTION   : readSectionsFromBuffer
 *
 * DESCRIPTION: read all jpeg sections of input buffer.
 *
 * PARAMETERS :
 *   @mem : buffer to read from Metadata Sections
 *   @buffer_size: buffer size
 *   @ReadMode: Read mode - all, jpeg or exif
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::ReadSectionsFromBuffer (unsigned char *buffer,
        unsigned int buffer_size, ReadMode_t ReadMode)
{
    int a;
    unsigned int pos = 0;
    int HaveCom = 0;
    mSectionsAllocated = 10;

    mSections = (Sections_t *)malloc(sizeof(Sections_t) * mSectionsAllocated);

    if (!buffer) {
        printf("Input buffer is null\n");
        return BAD_VALUE;
    }

    if (buffer_size < 1) {
        printf("Input size is 0\n");
        return BAD_VALUE;
    }

    a = (int) buffer[pos++];

    if (a != 0xff || buffer[pos++] != M_SOI){
        printf("No valid image\n");
        return BAD_VALUE;
    }

    for(;;){
        int itemlen;
        int marker = 0;
        int ll,lh;
        unsigned char * Data;

        CheckSectionsAllocated();

        for (a = 0; a <= 16; a++){
            marker = buffer[pos++];
            if (marker != 0xff) break;

            if (a >= 16){
                fprintf(stderr,"too many padding bytes\n");
                return BAD_VALUE;
            }
        }

        mSections[mSectionsRead].Type = marker;

        // Read the length of the section.
        lh = buffer[pos++];
        ll = buffer[pos++];

        itemlen = (lh << 8) | ll;

        if (itemlen < 2) {
            ALOGE("invalid marker");
            return BAD_VALUE;
        }

        mSections[mSectionsRead].Size = itemlen;

        Data = (unsigned char *)malloc(itemlen);
        if (Data == NULL) {
            ALOGE("Could not allocate memory");
            return NO_MEMORY;
        }
        mSections[mSectionsRead].Data = Data;

        // Store first two pre-read bytes.
        Data[0] = (unsigned char)lh;
        Data[1] = (unsigned char)ll;

        if (pos+itemlen-2 > buffer_size) {
           ALOGE("Premature end of file?");
          return BAD_VALUE;
        }

        memcpy(Data+2, buffer+pos, itemlen-2); // Read the whole section.
        pos += itemlen-2;

        mSectionsRead += 1;

        switch(marker){

            case M_SOS:   // stop before hitting compressed data
                // If reading entire image is requested, read the rest of the
                // data.
                if (ReadMode & READ_IMAGE){
                    int size;
                    // Determine how much file is left.
                    size = buffer_size - pos;

                    if (size < 1) {
                        ALOGE("could not read the rest of the image");
                        return BAD_VALUE;
                    }
                    Data = (unsigned char *)malloc(size);
                    if (Data == NULL) {
                        ALOGE("%d: could not allocate data for entire "
                            "image size: %d", __LINE__, size);
                        return BAD_VALUE;
                    }

                    memcpy(Data, buffer+pos, size);

                    CheckSectionsAllocated();
                    mSections[mSectionsRead].Data = Data;
                    mSections[mSectionsRead].Size = size;
                    mSections[mSectionsRead].Type = PSEUDO_IMAGE_MARKER;
                    mSectionsRead ++;
                    mHaveAll = 1;
                }
                return NO_ERROR;

            case M_EOI:   // in case it's a tables-only JPEG stream
                ALOGE("No image in jpeg!\n");
                return BAD_VALUE;

            case M_COM: // Comment section
                if (HaveCom || ((ReadMode & READ_METADATA) == 0)){
                    // Discard this section.
                    free(mSections[--mSectionsRead].Data);
                }
                break;

            case M_JFIF:
                // Regular jpegs always have this tag, exif images have the
                // exif marker instead, althogh ACDsee will write images
                // with both markers.
                // this program will re-create this marker on absence of exif
                // marker.
                // hence no need to keep the copy from the file.
                if (ReadMode & READ_METADATA){
                    if (memcmp(Data+2, "JFIF", 4) == 0) {
                        break;
                    }
                    free(mSections[--mSectionsRead].Data);
                }
                break;

            case M_EXIF:
                // There can be different section using the same marker.
                if (ReadMode & READ_METADATA){
                    if (memcmp(Data+2, "Exif", 4) == 0){
                        break;
                    }else if (memcmp(Data+2, "http:", 5) == 0){
                        // Change tag for internal purposes.
                        mSections[mSectionsRead-1].Type = M_XMP;
                        break;
                    }
                }
                // Oterwise, discard this section.
                free(mSections[--mSectionsRead].Data);
                break;

            case M_IPTC:
                if (ReadMode & READ_METADATA){
                    // Note: We just store the IPTC section.
                    // Its relatively straightforward
                    // and we don't act on any part of it,
                    // so just display it at parse time.
                }else{
                    free(mSections[--mSectionsRead].Data);
                }
                break;

            case M_SOF0:
            case M_SOF1:
            case M_SOF2:
            case M_SOF3:
            case M_SOF5:
            case M_SOF6:
            case M_SOF7:
            case M_SOF9:
            case M_SOF10:
            case M_SOF11:
            case M_SOF13:
            case M_SOF14:
            case M_SOF15:
                break;
            default:
                // Skip any other sections.
                break;
        }
    }
    return NO_ERROR;
}

void CameraContext::CheckSectionsAllocated(void)
{
    if (mSectionsRead > mSectionsAllocated){
        ALOGE("allocation screw up");
    }
    if (mSectionsRead >= mSectionsAllocated){
        mSectionsAllocated += mSectionsAllocated +1;
        mSections = (Sections_t *)realloc(mSections,
            sizeof(Sections_t) * mSectionsAllocated);
        if (mSections == NULL){
            ALOGE("could not allocate data for entire image");
        }
    }
}

/*===========================================================================
 * FUNCTION   : findSection
 *
 * DESCRIPTION: find the desired Section of the JPEG buffer.
 *
 * PARAMETERS :
 *  @SectionType: Section type
 *
 * RETURN     : return the found section

 *==========================================================================*/
CameraContext::Sections_t *CameraContext::FindSection(int SectionType)
{
    int a;

    for (a = 0; a < mSectionsRead; a++){
        if (mSections[a].Type == SectionType){
            return &mSections[a];
        }
    }
    // Could not be found.
    return NULL;
}


/*===========================================================================
 * FUNCTION   : DiscardData
 *
 * DESCRIPTION: DiscardData
 *
 * PARAMETERS : none
 *
 * RETURN     : none

 *==========================================================================*/
void CameraContext::DiscardData()
{
    int a;

    for (a = 0; a < mSectionsRead; a++){
        free(mSections[a].Data);
    }

    mSectionsRead = 0;
    mHaveAll = 0;
}

/*===========================================================================
 * FUNCTION   : DiscardSections
 *
 * DESCRIPTION: Discard allocated sections
 *
 * PARAMETERS : none
 *
 * RETURN     : none

 *==========================================================================*/
void CameraContext::DiscardSections()
{
    free(mSections);
    mSectionsAllocated = 0;
    mHaveAll = 0;
}

/*===========================================================================
 * FUNCTION   : notify
 *
 * DESCRIPTION: notify callback
 *
 * PARAMETERS :
 *   @msgType : type of callback
 *   @ext1: extended parameters
 *   @ext2: extended parameters
 *
 * RETURN     : None
 *==========================================================================*/
void CameraContext::notify(int32_t msgType, int32_t ext1, int32_t ext2)
{
    printf("Notify cb: %d %d %d\n", msgType, ext1, ext2);

    if ( msgType & CAMERA_MSG_FOCUS ) {
        printf("AutoFocus %s \n",
               (ext1) ? "OK" : "FAIL");
    }

    if ( msgType & CAMERA_MSG_SHUTTER ) {
        printf("Shutter done \n");
    }

    if ( msgType & CAMERA_MSG_ERROR) {
        printf("Camera Test CAMERA_MSG_ERROR\n");
        stopPreview();
        closeCamera();
    }
}

/*===========================================================================
 * FUNCTION   : postData
 *
 * DESCRIPTION: handles data callbacks
 *
 * PARAMETERS :
 *   @msgType : type of callback
 *   @dataPtr: buffer data
 *   @metadata: additional metadata where available
 *
 * RETURN     : None
 *==========================================================================*/
void CameraContext::postData(int32_t msgType,
                             const sp<IMemory>& dataPtr,
                             camera_frame_metadata_t *metadata)
{
    Size currentPictureSize = mSupportedPictureSizes.itemAt(
        mCurrentPictureSizeIdx);
    unsigned char *buff = NULL;
    int size;
    status_t ret = 0;

    memset(&mJEXIFSection, 0, sizeof(mJEXIFSection)),

    printf("Data cb: %d\n", msgType);

    if ( msgType & CAMERA_MSG_PREVIEW_FRAME ) {
        previewCallback(dataPtr);
    }

    if ( msgType & CAMERA_MSG_RAW_IMAGE ) {
        printf("RAW done \n");
    }

    if (msgType & CAMERA_MSG_POSTVIEW_FRAME) {
        printf("Postview frame \n");
    }

    if (msgType & CAMERA_MSG_COMPRESSED_IMAGE ) {
        String8 jpegPath;
        jpegPath = jpegPath.format("/sdcard/img_%d.jpg", JpegIdx);
        if (!mPiPCapture) {
            // Normal capture case
            printf("JPEG done\n");
            saveFile(dataPtr, jpegPath);
            JpegIdx++;
        } else {
            // PiP capture case
            SkFILEWStream *wStream;
            skBMtmp[mPiPIdx] = decodeJPEG(dataPtr);

            mWidthTmp[mPiPIdx] = currentPictureSize.width;
            mHeightTmp[mPiPIdx] = currentPictureSize.height;
            PiPPtrTmp[mPiPIdx] = dataPtr;
            // If there are two jpeg buffers
            if (mPiPIdx == 1) {
                printf("PiP done\n");

                // Find the the capture with higher width and height and read
                // its jpeg sections
                if ((mWidthTmp[0]*mHeightTmp[0]) >
                        (mWidthTmp[1]*mHeightTmp[1])) {
                    buff = (unsigned char *)PiPPtrTmp[0]->pointer();
                    size = PiPPtrTmp[0]->size();
                } else if ((mWidthTmp[0]*mHeightTmp[0]) <
                        (mWidthTmp[1]*mHeightTmp[1])) {
                    buff = (unsigned char *)PiPPtrTmp[1]->pointer();
                    size = PiPPtrTmp[1]->size();
                } else {
                    printf("Cannot take PiP. Images are with the same width"
                            " and height size!!!\n");
                    return;
                }

                if (buff != NULL && size != 0) {
                    ret = ReadSectionsFromBuffer(buff, size, READ_ALL);
                    if (ret != NO_ERROR) {
                        printf("Cannot read sections from buffer\n");
                        DiscardData();
                        DiscardSections();
                        return;
                    }

                    mJEXIFTmp = FindSection(M_EXIF);
                    mJEXIFSection = *mJEXIFTmp;
                    mJEXIFSection.Data =
                        (unsigned char*)malloc(mJEXIFTmp->Size);
                    memcpy(mJEXIFSection.Data,
                        mJEXIFTmp->Data, mJEXIFTmp->Size);
                    DiscardData();
                    DiscardSections();

                    wStream = new SkFILEWStream(jpegPath.string());
                    skBMDec = PiPCopyToOneFile(skBMtmp[0], skBMtmp[1]);
                    if (encodeJPEG(wStream, skBMDec, jpegPath) != false) {
                        printf("%s():%d:: Failed during jpeg encode\n",
                            __FUNCTION__,__LINE__);
                        return;
                    }
                    mPiPIdx = 0;
                    JpegIdx++;
                    delete wStream;
                }
            } else {
                mPiPIdx++;
            }
            disablePiPCapture();
        }
    }

    if ( ( msgType & CAMERA_MSG_PREVIEW_METADATA ) &&
         ( NULL != metadata ) ) {
        printf("Face detected %d \n", metadata->number_of_faces);
    }

    signalFinished();
}

/*===========================================================================
 * FUNCTION   : postDataTimestamp
 *
 * DESCRIPTION: handles recording callbacks
 *
 * PARAMETERS :
 *   @timestamp : timestamp of buffer
 *   @msgType : type of buffer
 *   @dataPtr : buffer data
 *
 * RETURN     : None
 *==========================================================================*/
void CameraContext::postDataTimestamp(nsecs_t timestamp,
                                      int32_t msgType,
                                      const sp<IMemory>& dataPtr)
{
    printf("Recording cb: %d %lld %p\n", msgType, timestamp, dataPtr.get());
}

/*===========================================================================
 * FUNCTION   : printSupportedParams
 *
 * DESCRIPTION: dump common supported parameters
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
void CameraContext::printSupportedParams()
{
    printf("\n\r\tSupported Cameras: %s",
           mParams.get("camera-indexes")?
               mParams.get("camera-indexes") : "NULL");
    printf("\n\r\tSupported Picture Sizes: %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES)?
           mParams.get(
               CameraParameters::KEY_SUPPORTED_PICTURE_SIZES) : "NULL");
    printf("\n\r\tSupported Picture Formats: %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS)?
           mParams.get(
               CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS) : "NULL");
    printf("\n\r\tSupported Preview Sizes: %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES)?
           mParams.get(
               CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES) : "NULL");
    printf("\n\r\tSupported Preview Formats: %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS)?
           mParams.get(
               CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS) : "NULL");
    printf("\n\r\tSupported Preview Frame Rates: %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES)?
           mParams.get(
               CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES) : "NULL");
    printf("\n\r\tSupported Thumbnail Sizes: %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES)?
           mParams.get(
               CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES) : "NULL");
    printf("\n\r\tSupported Whitebalance Modes: %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE)?
           mParams.get(
               CameraParameters::KEY_SUPPORTED_WHITE_BALANCE) : "NULL");
    printf("\n\r\tSupported Effects: %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_EFFECTS)?
           mParams.get(CameraParameters::KEY_SUPPORTED_EFFECTS) : "NULL");
    printf("\n\r\tSupported Scene Modes: %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_SCENE_MODES)?
           mParams.get(CameraParameters::KEY_SUPPORTED_SCENE_MODES) : "NULL");
    printf("\n\r\tSupported Focus Modes: %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_FOCUS_MODES)?
           mParams.get(CameraParameters::KEY_SUPPORTED_FOCUS_MODES) : "NULL");
    printf("\n\r\tSupported Antibanding Options: %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_ANTIBANDING)?
           mParams.get(CameraParameters::KEY_SUPPORTED_ANTIBANDING) : "NULL");
    printf("\n\r\tSupported Flash Modes: %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES)?
           mParams.get(CameraParameters::KEY_SUPPORTED_FLASH_MODES) : "NULL");
    printf("\n\r\tSupported Focus Areas: %d",
           mParams.getInt(CameraParameters::KEY_MAX_NUM_FOCUS_AREAS));
    printf("\n\r\tSupported FPS ranges : %s",
           mParams.get(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE)?
           mParams.get(
               CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE) : "NULL");
    printf("\n\r\tFocus Distances: %s \n",
           mParams.get(CameraParameters::KEY_FOCUS_DISTANCES)?
           mParams.get(CameraParameters::KEY_FOCUS_DISTANCES) : "NULL");
}

/*===========================================================================
 * FUNCTION   : createPreviewSurface
 *
 * DESCRIPTION: helper function for creating preview surfaces
 *
 * PARAMETERS :
 *   @width : preview width
 *   @height: preview height
 *   @pixFormat : surface pixelformat
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::createPreviewSurface(unsigned int width,
                                             unsigned int height,
                                             int32_t pixFormat)
{
    int ret = NO_ERROR;
    DisplayInfo dinfo;
    sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(
                        ISurfaceComposer::eDisplayIdMain));
    SurfaceComposerClient::getDisplayInfo(display, &dinfo);
    unsigned int previewWidth, previewHeight;

    if ( dinfo.w < width ) {
        previewWidth = dinfo.w;
    } else {
        previewWidth = width;
    }

    if ( dinfo.h < height ) {
        previewHeight = dinfo.h;
    } else {
        previewHeight = height;
    }

    mClient = new SurfaceComposerClient();

    if ( NULL == mClient.get() ) {
        printf("Unable to establish connection to Surface Composer \n");
        return NO_INIT;
    }

    mSurfaceControl = mClient->createSurface(String8("QCamera_Test"),
                                             previewWidth,
                                             previewHeight,
                                             pixFormat,
                                             0);
    if ( NULL == mSurfaceControl.get() ) {
        printf("Unable to create preview surface \n");
        return NO_INIT;
    }

    mPreviewSurface = mSurfaceControl->getSurface();
    if ( NULL != mPreviewSurface.get() ) {
        mClient->openGlobalTransaction();
        ret |= mSurfaceControl->setLayer(0x7fffffff);
        if ( mCameraIndex == 0 )
            ret |= mSurfaceControl->setPosition(0, 0);
        else
            ret |= mSurfaceControl->setPosition(
                dinfo.w - previewWidth, dinfo.h - previewHeight);

        ret |= mSurfaceControl->setSize(previewWidth, previewHeight);
        ret |= mSurfaceControl->show();
        mClient->closeGlobalTransaction();

        if ( NO_ERROR != ret ) {
            printf("Preview surface configuration failed! \n");
        }
    } else {
        ret = NO_INIT;
    }

    return ret;
}

/*===========================================================================
 * FUNCTION   : destroyPreviewSurface
 *
 * DESCRIPTION: closes previously open preview surface
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::destroyPreviewSurface()
{
    if ( NULL != mPreviewSurface.get() ) {
        mPreviewSurface.clear();
    }

    if ( NULL != mSurfaceControl.get() ) {
        mSurfaceControl->clear();
        mSurfaceControl.clear();
    }

    if ( NULL != mClient.get() ) {
        mClient->dispose();
        mClient.clear();
    }

    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : CameraContext
 *
 * DESCRIPTION: camera context constructor
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
CameraContext::CameraContext(int cameraIndex) :
    mCameraIndex(cameraIndex),
    mResizePreview(true),
    mHardwareActive(false),
    mPreviewRunning(false),
    mRecordRunning(false),
    mVideoFd(-1),
    mVideoIdx(0),
    mRecordingHint(false),
    mDoPrintMenu(true),
    mPiPCapture(false),
    mfmtMultiplier(1),
    mSectionsRead(false),
    mSectionsAllocated(false),
    mSections(NULL),
    mJEXIFTmp(NULL),
    mHaveAll(false),
    mCamera(NULL),
    mClient(NULL),
    mSurfaceControl(NULL),
    mPreviewSurface(NULL),
    mInUse(false)
{
    mRecorder = new MediaRecorder();
}

/*===========================================================================
 * FUNCTION   : ~CameraContext
 *
 * DESCRIPTION: camera context destructor
 *
 * PARAMETERS : None
 *
 * RETURN     : None
 *==========================================================================*/
CameraContext::~CameraContext()
{
    stopPreview();
    closeCamera();
}

/*===========================================================================
 * FUNCTION   : openCamera
 *
 * DESCRIPTION: connects to and initializes camera
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t  CameraContext::openCamera()
{
    useLock();

    if ( NULL != mCamera.get() ) {
        printf("Camera already open! \n");
        return NO_ERROR;
    }

    printf("openCamera(camera_index=%d)\n", mCameraIndex);

#ifndef USE_JB_MR1

    String16 packageName("CameraTest");

    mCamera = Camera::connect(mCameraIndex,
                              packageName,
                              Camera::USE_CALLING_UID);

#else

    mCamera = Camera::connect(mCameraIndex);

#endif

    if ( NULL == mCamera.get() ) {
        printf("Unable to connect to CameraService\n");
        return NO_INIT;
    }

    mParams = mCamera->getParameters();
    mParams.getSupportedPreviewSizes(mSupportedPreviewSizes);
    mParams.getSupportedPictureSizes(mSupportedPictureSizes);
    mParams.getSupportedVideoSizes(mSupportedVideoSizes);

    mCurrentPictureSizeIdx = mSupportedPictureSizes.size() / 2;
    mCurrentPreviewSizeIdx = mSupportedPreviewSizes.size() / 2;
    mCurrentVideoSizeIdx   = mSupportedVideoSizes.size() / 2;

    mCamera->setListener(this);
    mHardwareActive = true;

    signalFinished();

    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : getNumberOfCameras
 *
 * DESCRIPTION: returns the number of supported camera by the system
 *
 * PARAMETERS : None
 *
 * RETURN     : supported camera count
 *==========================================================================*/
int CameraContext::getNumberOfCameras()
{
    int ret = -1;

    if ( NULL != mCamera.get() ) {
        ret = mCamera->getNumberOfCameras();
    }

    return ret;
}

/*===========================================================================
 * FUNCTION   : closeCamera
 *
 * DESCRIPTION: closes a previously the initialized camera reference
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::closeCamera()
{
    useLock();
    if ( NULL == mCamera.get() ) {
        return NO_INIT;
    }

    mCamera->disconnect();
    mCamera.clear();

    mRecorder->init();
    mRecorder->close();
    mRecorder->release();
    mRecorder.clear();

    mHardwareActive = false;
    mPreviewRunning = false;
    mRecordRunning = false;

    signalFinished();
    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : startPreview
 *
 * DESCRIPTION: starts camera preview
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::startPreview()
{
    useLock();

    int ret = NO_ERROR;
    int previewWidth, previewHeight;
    Size currentPreviewSize = mSupportedPreviewSizes.itemAt(
        mCurrentPreviewSizeIdx);
    Size currentPictureSize = mSupportedPictureSizes.itemAt(
        mCurrentPictureSizeIdx);
    Size currentVideoSize   = mSupportedVideoSizes.itemAt(
        mCurrentVideoSizeIdx);

#ifndef USE_JB_MR1

    sp<IGraphicBufferProducer> gbp;

#endif

    if (!mHardwareActive ) {
        printf("Camera not active! \n");
        return NO_INIT;
    }

    if (mPreviewRunning) {
        printf("Preview is already running! \n");
        signalFinished();
        return NO_ERROR;
    }

    if (mResizePreview) {
        mPreviewRunning = false;

        if ( mRecordingHint ) {
            previewWidth = currentVideoSize.width;
            previewHeight = currentVideoSize.height;
        } else {
            previewWidth = currentPreviewSize.width;
            previewHeight = currentPreviewSize.height;
        }

        ret = createPreviewSurface(previewWidth,
                                   previewHeight,
                                   HAL_PIXEL_FORMAT_YCrCb_420_SP);
        if (  NO_ERROR != ret ) {
            printf("Error while creating preview surface\n");
            return ret;
        }


        mParams.setPreviewSize(previewWidth, previewHeight);
        mParams.setPictureSize(currentPictureSize.width,
            currentPictureSize.height);
        mParams.setVideoSize(
            currentVideoSize.width, currentVideoSize.height);

        ret |= mCamera->setParameters(mParams.flatten());

#ifndef USE_JB_MR1

        gbp = mPreviewSurface->getIGraphicBufferProducer();
        ret |= mCamera->setPreviewTarget(gbp);

#else

        ret |= mCamera->setPreviewDisplay(mPreviewSurface);

#endif
        mResizePreview = false;
    }

    if ( !mPreviewRunning ) {
        ret |= mCamera->startPreview();
        if ( NO_ERROR != ret ) {
            printf("Preview start failed! \n");
            return ret;
        }

        mPreviewRunning = true;
    }

    signalFinished();

    return ret;
}

/*===========================================================================
 * FUNCTION   : autoFocus
 *
 * DESCRIPTION: Triggers autofocus
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::autoFocus()
{
    useLock();
    status_t ret = NO_ERROR;

    if ( mPreviewRunning ) {
        ret = mCamera->autoFocus();
    }

    signalFinished();
    return ret;
}

/*===========================================================================
 * FUNCTION   : enablePreviewCallbacks
 *
 * DESCRIPTION: Enables preview callback messages
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::enablePreviewCallbacks()
{
    useLock();
    if ( mHardwareActive ) {
        mCamera->setPreviewCallbackFlags(
            CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK);
    }

    signalFinished();
    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : takePicture
 *
 * DESCRIPTION: triggers image capture
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::takePicture()
{
    status_t ret = NO_ERROR;

    useLock(); // Unlocked in jpeg callback

    if ( mPreviewRunning ) {
        ret = mCamera->takePicture(
            CAMERA_MSG_COMPRESSED_IMAGE|
            CAMERA_MSG_RAW_IMAGE);
        if (!mRecordingHint) {
            mPreviewRunning = false;
        }
    } else {
        printf("Please resume/start the preview before taking a picture!\n");
        signalFinished(); //Unlock in case preview is not running
    }
    return ret;
}


status_t CameraContext::configureRecorder()
{
    useLock();
    status_t ret = NO_ERROR;

    mParams.unflatten(mCamera->getParameters());
    int width, height;
    mParams.getVideoSize(&width, &height);

    mResizePreview = true;
    mParams.set("recording-hint", "true");
    mRecordingHint = true;
    mCamera->setParameters(mParams.flatten());


    ret = mRecorder->setParameters(
        String8("video-param-encoding-bitrate=64000"));
    if ( ret != NO_ERROR ) {
        ERROR("Could not configure recorder (%d)", ret);
        return ret;
    }

    ret = mRecorder->setCamera(
        mCamera->remote(), mCamera->getRecordingProxy());
    if ( ret != NO_ERROR ) {
        ERROR("Could not set camera (%d)", ret);
        return ret;
    }
    ret = mRecorder->setVideoSource(VIDEO_SOURCE_CAMERA);
    if ( ret != NO_ERROR ) {
        ERROR("Could not set video soruce (%d)", ret);
        return ret;
    }
    ret = mRecorder->setAudioSource(AUDIO_SOURCE_DEFAULT);
    if ( ret != NO_ERROR ) {
        ERROR("Could not set audio source (%d)", ret);
        return ret;
    }
    ret = mRecorder->setOutputFormat(OUTPUT_FORMAT_DEFAULT);
    if ( ret != NO_ERROR ) {
        ERROR("Could not set output format (%d)", ret);
        return ret;
    }

    char fileName[100];

    sprintf(fileName, "/sdcard/vid_cam%d_%dx%d_%d.mpeg", mCameraIndex,
            width, height, mVideoIdx++);

    if ( mVideoFd < 0 ) {
        mVideoFd = open(fileName, O_CREAT | O_RDWR );
    }

    if ( mVideoFd < 0 ) {
        ERROR("Could not open video file for writing %s!", fileName);
        return UNKNOWN_ERROR;
    }

    ret = mRecorder->setOutputFile(mVideoFd, 0, 0);
    if ( ret != NO_ERROR ) {
        ERROR("Could not set output file (%d)", ret);
        return ret;
    }
    ret = mRecorder->setVideoFrameRate(30);
    if ( ret != NO_ERROR ) {
        ERROR("Could not set video frame rate (%d)", ret);
        return ret;
    }

    ret = mRecorder->setVideoSize(width, height);
    if ( ret  != NO_ERROR ) {
        ERROR("Could not set video size %dx%d", width, height);
        return ret;
    }

    ret = mRecorder->setVideoEncoder(VIDEO_ENCODER_DEFAULT);
    if ( ret != NO_ERROR ) {
        ERROR("Could not set video encoder (%d)", ret);
        return ret;
    }
    ret = mRecorder->setAudioEncoder(AUDIO_ENCODER_DEFAULT);
    if ( ret != NO_ERROR ) {
        ERROR("Could not set audio encoder (%d)", ret);
        return ret;
    }

    signalFinished();
    return ret;
}

status_t CameraContext::unconfigureRecorder()
{
    useLock();

    if ( !mRecordRunning ) {
        mResizePreview = true;
        mParams.set("recording-hint", "false");
        mRecordingHint = false;
        mCamera->setParameters(mParams.flatten());
    }

    signalFinished();
    return NO_ERROR;
}


/*===========================================================================
 * FUNCTION   : startRecording
 *
 * DESCRIPTION: triggers start recording
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::startRecording()
{
    useLock();
    status_t ret = NO_ERROR;

    if ( mPreviewRunning ) {

        mCamera->unlock();

        ret = mRecorder->prepare();
        if ( ret != NO_ERROR ) {
            ERROR("Could not prepare recorder");
            return ret;
        }

        ret = mRecorder->start();
        if ( ret != NO_ERROR ) {
            ERROR("Could not start recorder");
            return ret;
        }

        mRecordRunning = true;
    }
    signalFinished();
    return ret;
}

/*===========================================================================
 * FUNCTION   : stopRecording
 *
 * DESCRIPTION: triggers start recording
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::stopRecording()
{
    useLock();
    status_t ret = NO_ERROR;

    if ( mRecordRunning ) {
        mRecorder->stop();

        close(mVideoFd);
        mVideoFd = -1;
        mRecordRunning = false;
    }

    signalFinished();

    return ret;
}

/*===========================================================================
 * FUNCTION   : stopPreview
 *
 * DESCRIPTION: stops camera preview
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::stopPreview()
{
    useLock();
    status_t ret = NO_ERROR;

    if ( mHardwareActive ) {
        mCamera->stopPreview();
        ret = destroyPreviewSurface();
    }

    mPreviewRunning  = false;
    mResizePreview = true;

    signalFinished();

    return ret;
}

/*===========================================================================
 * FUNCTION   : resumePreview
 *
 * DESCRIPTION: resumes camera preview after image capture
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::resumePreview()
{
    useLock();
    status_t ret = NO_ERROR;

    if ( mHardwareActive ) {
        ret = mCamera->startPreview();
        mPreviewRunning = true;
    } else {
        ret = NO_INIT;
    }

    signalFinished();
    return ret;
}

/*===========================================================================
 * FUNCTION   : nextPreviewSize
 *
 * DESCRIPTION: Iterates through all supported preview sizes.
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::nextPreviewSize()
{
    useLock();
    if ( mHardwareActive ) {
        mCurrentPreviewSizeIdx += 1;
        mCurrentPreviewSizeIdx %= mSupportedPreviewSizes.size();
        Size previewSize = mSupportedPreviewSizes.itemAt(
            mCurrentPreviewSizeIdx);
        mParams.setPreviewSize(previewSize.width,
                               previewSize.height);
        mResizePreview = true;

        if ( mPreviewRunning ) {
            mCamera->stopPreview();
            mCamera->setParameters(mParams.flatten());
            mCamera->startPreview();
        } else {
            mCamera->setParameters(mParams.flatten());
        }
    }

    signalFinished();
    return NO_ERROR;
}


/*===========================================================================
 * FUNCTION   : setPreviewSize
 *
 * DESCRIPTION: Sets exact preview size if supported
 *
 * PARAMETERS : format size in the form of WIDTHxHEIGHT
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::setPreviewSize(const char *format)
{
    useLock();
    if ( mHardwareActive ) {
        int newHeight;
        int newWidth;
        sscanf(format, "%dx%d", &newWidth, &newHeight);

        unsigned int i;
        for (i = 0; i < mSupportedPreviewSizes.size(); ++i) {
            Size previewSize = mSupportedPreviewSizes.itemAt(i);
            if ( newWidth == previewSize.width &&
                 newHeight == previewSize.height )
            {
                break;
            }

        }
        if ( i == mSupportedPreviewSizes.size())
        {
            printf("Preview size %dx%d not supported !\n",
                newWidth, newHeight);
            return INVALID_OPERATION;
        }

        mParams.setPreviewSize(newWidth,
                               newHeight);
        mResizePreview = true;

        if ( mPreviewRunning ) {
            mCamera->stopPreview();
            mCamera->setParameters(mParams.flatten());
            mCamera->startPreview();
        } else {
            mCamera->setParameters(mParams.flatten());
        }
    }

    signalFinished();
    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : getCurrentPreviewSize
 *
 * DESCRIPTION: queries the currently configured preview size
 *
 * PARAMETERS :
 *  @previewSize : preview size currently configured
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::getCurrentPreviewSize(Size &previewSize)
{
    useLock();
    if ( mHardwareActive ) {
        previewSize = mSupportedPreviewSizes.itemAt(mCurrentPreviewSizeIdx);
    }
    signalFinished();
    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : nextPictureSize
 *
 * DESCRIPTION: Iterates through all supported picture sizes.
 *
 * PARAMETERS : None
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::nextPictureSize()
{
    useLock();
    if ( mHardwareActive ) {
        mCurrentPictureSizeIdx += 1;
        mCurrentPictureSizeIdx %= mSupportedPictureSizes.size();
        Size pictureSize = mSupportedPictureSizes.itemAt(
            mCurrentPictureSizeIdx);
        mParams.setPictureSize(pictureSize.width,
            pictureSize.height);
        mCamera->setParameters(mParams.flatten());
    }
    signalFinished();
    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : setPictureSize
 *
 * DESCRIPTION: Sets exact preview size if supported
 *
 * PARAMETERS : format size in the form of WIDTHxHEIGHT
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::setPictureSize(const char *format)
{
    useLock();
    if ( mHardwareActive ) {
        int newHeight;
        int newWidth;
        sscanf(format, "%dx%d", &newWidth, &newHeight);

        unsigned int i;
        for (i = 0; i < mSupportedPictureSizes.size(); ++i) {
            Size PictureSize = mSupportedPictureSizes.itemAt(i);
            if ( newWidth == PictureSize.width &&
                 newHeight == PictureSize.height )
            {
                break;
            }

        }
        if ( i == mSupportedPictureSizes.size())
        {
            printf("Preview size %dx%d not supported !\n",
                newWidth, newHeight);
            return INVALID_OPERATION;
        }

        mParams.setPictureSize(newWidth,
                               newHeight);
        mCamera->setParameters(mParams.flatten());
    }

    signalFinished();
    return NO_ERROR;
}

status_t CameraContext::nextVideoSize()
{
    useLock();
    if ( mHardwareActive ) {
        mCurrentVideoSizeIdx += 1;
        mCurrentVideoSizeIdx %= mSupportedVideoSizes.size();
        Size videoSize = mSupportedVideoSizes.itemAt(mCurrentVideoSizeIdx);
        mParams.setVideoSize(videoSize.width,
                             videoSize.height);
        mCamera->setParameters(mParams.flatten());
    }
    signalFinished();
    return NO_ERROR;
}

status_t CameraContext::setVideoSize(const char *format)
{
    useLock();
    if ( mHardwareActive ) {
        int newHeight;
        int newWidth;
        sscanf(format, "%dx%d", &newWidth, &newHeight);

        unsigned int i;
        for (i = 0; i < mSupportedVideoSizes.size(); ++i) {
            Size PictureSize = mSupportedVideoSizes.itemAt(i);
            if ( newWidth == PictureSize.width &&
                 newHeight == PictureSize.height )
            {
                break;
            }

        }
        if ( i == mSupportedVideoSizes.size())
        {
            printf("Preview size %dx%d not supported !\n",
                newWidth, newHeight);
            return INVALID_OPERATION;
        }

        mParams.setVideoSize(newWidth,
                             newHeight);
        mCamera->setParameters(mParams.flatten());
    }

    signalFinished();
    return NO_ERROR;
}

status_t CameraContext::getCurrentVideoSize(Size &videoSize)
{
    useLock();
    if ( mHardwareActive ) {
        videoSize = mSupportedVideoSizes.itemAt(mCurrentVideoSizeIdx);
    }
    signalFinished();
    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : getCurrentPictureSize
 *
 * DESCRIPTION: queries the currently configured picture size
 *
 * PARAMETERS :
 *  @pictureSize : picture size currently configured
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- success
 *              none-zero failure code
 *==========================================================================*/
status_t CameraContext::getCurrentPictureSize(Size &pictureSize)
{
    useLock();
    if ( mHardwareActive ) {
        pictureSize = mSupportedPictureSizes.itemAt(mCurrentPictureSizeIdx);
    }
    signalFinished();
    return NO_ERROR;
}

}; //namespace qcamera ends here

using namespace qcamera;

/*===========================================================================
 * FUNCTION   : printMenu
 *
 * DESCRIPTION: prints the available camera options
 *
 * PARAMETERS :
 *  @currentCamera : camera context currently being used
 *
 * RETURN     : None
 *==========================================================================*/
void CameraContext::printMenu(sp<CameraContext> currentCamera)
{
    if ( !mDoPrintMenu ) return;
    Size currentPictureSize, currentPreviewSize;

    assert(currentCamera.get());

    currentCamera->getCurrentPictureSize(currentPictureSize);
    currentCamera->getCurrentPreviewSize(currentPreviewSize);

    printf("\n\n=========== FUNCTIONAL TEST MENU ===================\n\n");

    printf(" \n\nSTART / STOP / GENERAL SERVICES \n");
    printf(" -----------------------------\n");
    printf("   %c. Switch camera - Current Index: %d\n",
            Interpreter::SWITCH_CAMERA_CMD,
            currentCamera->getCameraIndex());
    printf("   %c. Resume Preview after capture \n",
            Interpreter::RESUME_PREVIEW_CMD);
    printf("   %c. Quit \n",
            Interpreter::EXIT_CMD);
    printf("   %c. Camera Capability Dump",
            Interpreter::DUMP_CAPS_CMD);

    printf(" \n\n PREVIEW SUB MENU \n");
    printf(" -----------------------------\n");
    printf("   %c. Start Preview\n",
            Interpreter::START_PREVIEW_CMD);
    printf("   %c. Stop Preview\n",
            Interpreter::STOP_PREVIEW_CMD);
    printf("   %c. Preview size:  %dx%d\n",
           Interpreter::CHANGE_PREVIEW_SIZE_CMD,
           currentPreviewSize.width,
           currentPreviewSize.height);
    printf("   %c. Start Recording\n",
            Interpreter::START_RECORD_CMD);
    printf("   %c. Stop Recording\n",
            Interpreter::STOP_RECORD_CMD);
    printf("   %c. Enable preview frames\n",
            Interpreter::ENABLE_PRV_CALLBACKS_CMD);
    printf("   %c. Trigger autofocus \n",
            Interpreter::AUTOFOCUS_CMD);

    printf(" \n\n IMAGE CAPTURE SUB MENU \n");
    printf(" -----------------------------\n");
    printf("   %c. Take picture/Full Press\n",
            Interpreter::TAKEPICTURE_CMD);
    printf("   %c. Take picture in picture\n",
            Interpreter::TAKEPICTURE_IN_PICTURE_CMD);
    printf("   %c. Picture size:  %dx%d\n",
           Interpreter::CHANGE_PICTURE_SIZE_CMD,
           currentPictureSize.width,
           currentPictureSize.height);

    printf("\n");
    printf("   Choice: ");
}

void CameraContext::enablePrintPreview()
{
    mDoPrintMenu = true;
}

void CameraContext::disablePrintPreview()
{
    mDoPrintMenu = false;
}

void CameraContext::enablePiPCapture()
{
    mPiPCapture = true;
}

void CameraContext::disablePiPCapture()
{
    mPiPCapture = false;
}

Interpreter::Interpreter(const char *file)
    : mCmdIndex(0)
    , mScript(NULL)
{
    if (!file){
        printf("no File Given\n");
        mUseScript = false;
        return;
    }

    FILE *fh = fopen(file, "r");
    if ( !fh ) {
        printf("Could not open file %s\n", file);
        mUseScript = false;
        return;
    }

    fseek(fh, 0, SEEK_END);
    int len = ftell(fh);
    rewind(fh);

    if( !len ) {
        printf("Script file %s is empty !\n", file);
        fclose(fh);
        return;
    }

    mScript = new char[len + 1];
    if ( !mScript ) {
        fclose(fh);
        return;
    }

    fread(mScript, sizeof(char), len, fh);
    mScript[len] = '\0'; // ensure null terminated;
    fclose(fh);


    char *p1;
    char *p2;
    p1 = p2 = mScript;

    do {
        switch (*p1) {
        case '\0':
        case '|':
            p1++;
            break;
        case SWITCH_CAMERA_CMD:
        case RESUME_PREVIEW_CMD:
        case START_PREVIEW_CMD:
        case STOP_PREVIEW_CMD:
        case CHANGE_PREVIEW_SIZE_CMD:
        case CHANGE_PICTURE_SIZE_CMD:
        case START_RECORD_CMD:
        case STOP_RECORD_CMD:
        case DUMP_CAPS_CMD:
        case AUTOFOCUS_CMD:
        case TAKEPICTURE_CMD:
        case TAKEPICTURE_IN_PICTURE_CMD:
        case ENABLE_PRV_CALLBACKS_CMD:
        case EXIT_CMD:
        case DELAY:
            p2 = p1;
            while( (p2 != (mScript + len)) && (*p2 != '|')) {
                p2++;
            }
            *p2 = '\0';
            if (p2 == (p1 + 1))
                mCommands.push_back(Command(
                    static_cast<Interpreter::Commands_e>(*p1)));
            else
                mCommands.push_back(Command(
                    static_cast<Interpreter::Commands_e>(*p1), (p1 + 1)));
            p1 = p2;
            break;
        default:
            printf("Invalid cmd %c \n", *p1);
            do {
                p1++;

            } while(*p1 != '|' && p1 != (mScript + len));

        }
    } while(p1 != (mScript + len));
    mUseScript = true;
}

Interpreter::~Interpreter()
{
    if ( mScript )
        delete[] mScript;

    mCommands.clear();
}

Interpreter::Command Interpreter::getCommand(
    sp<CameraContext> currentCamera)
{
    if( mUseScript ) {
        return mCommands[mCmdIndex++];
    } else {
        currentCamera->printMenu(currentCamera);
        return Interpreter::Command(
            static_cast<Interpreter::Commands_e>(getchar()));
    }
}


TestContext::TestContext()
    : mTestRunning(false)
    , mCurrentCameraIndex(0)
    , mInterpreter(NULL)
{
    sp<CameraContext> camera;
    int i = 0;

    ProcessState::self()->startThreadPool();

    do {
        camera = new CameraContext(i);
        if ( NULL == camera.get() ) {
            break;
        }

        status_t stat = camera->openCamera();
        if ( NO_ERROR != stat ) {
            printf("Error encountered Openging camera id : %d\n", i);
            break;
        }

        mAvailableCameras.add(camera);
        i++;
    } while ( i < camera->getNumberOfCameras() ) ;

    if (i < camera->getNumberOfCameras() ) {
        for (size_t j = 0; j < mAvailableCameras.size(); j++) {
            camera = mAvailableCameras.itemAt(j);
            camera->closeCamera();
            camera.clear();
        }

        mAvailableCameras.clear();
    }
}

TestContext::~TestContext()
{
    delete mInterpreter;

    for (size_t j = 0; j < mAvailableCameras.size(); j++) {
        sp<CameraContext> camera = mAvailableCameras.itemAt(j);
        camera->closeCamera();
        camera.clear();
    }

    mAvailableCameras.clear();
}

int32_t TestContext::GetCamerasNum()
{
    return mAvailableCameras.size();
}

status_t TestContext::AddScriptFromFile(const char *scriptFile)
{
    mInterpreter = new Interpreter(scriptFile);

    return NO_ERROR;
}

/*===========================================================================
 * FUNCTION   : functionalTest
 *
 * DESCRIPTION: queries and executes client supplied commands for testing a
 *              particular camera.
 *
 * PARAMETERS :
 *  @availableCameras : List with all cameras supported
 *
 * RETURN     : status_t type of status
 *              NO_ERROR  -- continue testing
 *              none-zero -- quit test
 *==========================================================================*/
status_t TestContext::FunctionalTest()
{
    status_t stat = NO_ERROR;

    assert(mAvailableCameras.size());

    if ( !mInterpreter )
        mInterpreter = new Interpreter();

    mTestRunning = true;

    while (mTestRunning) {
        sp<CameraContext> currentCamera =
            mAvailableCameras.itemAt(mCurrentCameraIndex);
        Interpreter::Command command =
            mInterpreter->getCommand(currentCamera);
        currentCamera->enablePrintPreview();

        switch (command.cmd) {
        case Interpreter::SWITCH_CAMERA_CMD:
        {
            mCurrentCameraIndex++;
            mCurrentCameraIndex %= mAvailableCameras.size();
            currentCamera = mAvailableCameras.itemAt(mCurrentCameraIndex);
        }
            break;

        case Interpreter::RESUME_PREVIEW_CMD:
        {
            stat = currentCamera->resumePreview();
        }
            break;

        case Interpreter::START_PREVIEW_CMD:
        {
            stat = currentCamera->startPreview();
        }
            break;

        case Interpreter::STOP_PREVIEW_CMD:
        {
            stat = currentCamera->stopPreview();
        }
            break;

        case Interpreter::CHANGE_PREVIEW_SIZE_CMD:
        {
            if ( command.arg )
                stat = currentCamera->setPreviewSize(command.arg);
            else
                stat = currentCamera->nextPreviewSize();
        }
            break;

        case Interpreter::CHANGE_PICTURE_SIZE_CMD:
        {
            if ( command.arg )
                stat = currentCamera->setPictureSize(command.arg);
            else
                stat = currentCamera->nextPictureSize();
        }
            break;

        case Interpreter::DUMP_CAPS_CMD:
        {
            currentCamera->printSupportedParams();
        }
            break;

        case Interpreter::AUTOFOCUS_CMD:
        {
            stat = currentCamera->autoFocus();
        }
            break;

        case Interpreter::TAKEPICTURE_CMD:
        {
            stat = currentCamera->takePicture();
        }
            break;

        case Interpreter::TAKEPICTURE_IN_PICTURE_CMD:
        {
            if (mAvailableCameras.size() == 2) {
                mSaveCurrentCameraIndex = mCurrentCameraIndex;
                for (size_t i = 0; i < mAvailableCameras.size(); i++) {
                    mCurrentCameraIndex = i;
                    currentCamera = mAvailableCameras.itemAt(
                        mCurrentCameraIndex);
                    currentCamera->enablePiPCapture();
                    stat = currentCamera->takePicture();
                }
                mCurrentCameraIndex = mSaveCurrentCameraIndex;
            } else {
                printf("Number of available sensors should be 2\n");
            }
        }
        break;

        case Interpreter::ENABLE_PRV_CALLBACKS_CMD:
        {
            stat = currentCamera->enablePreviewCallbacks();
        }
            break;

        case Interpreter::START_RECORD_CMD:
        {
            stat = currentCamera->stopPreview();
            stat = currentCamera->configureRecorder();
            stat = currentCamera->startPreview();

            stat = currentCamera->startRecording();
        }
            break;

        case Interpreter::STOP_RECORD_CMD:
        {
            stat = currentCamera->stopRecording();

            stat = currentCamera->stopPreview();
            stat = currentCamera->unconfigureRecorder();
            stat = currentCamera->startPreview();
        }
            break;

        case Interpreter::EXIT_CMD:
        {
            currentCamera->stopPreview();
            mTestRunning = false;
        }
            break;
        case Interpreter::DELAY:
        {
            if ( command.arg )
                usleep(1000 * atoi(command.arg));
        }
            break;
        default:
        {
            currentCamera->disablePrintPreview();
        }
            break;
        }
        printf("Command status 0x%x \n", stat);
    }

    return NO_ERROR;
}

int
main(int argc, char *argv[])
{
    TestContext ctx;

    if (argc > 1) {
        if ( ctx.AddScriptFromFile((const char *)argv[1]) ) {
            printf("Could not add script file... "
                "continuing in normal menu mode! \n");
        }
    }

    ctx.FunctionalTest();

    return 0;
}
