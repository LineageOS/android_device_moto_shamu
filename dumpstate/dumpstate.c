/*
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dumpstate.h>

void dumpstate_board()
{
    dump_file("TZ ramoops annotation", "/sys/fs/pstore/annotate-ramoops");
    dump_file("cpuinfo", "/proc/cpuinfo");
    dump_file("Interrupts", "/proc/interrupts");
    dump_file("Power Management Stats", "/proc/msm_pm_stats");
    dump_file("RPM Stats", "/d/rpm_stats");
    run_command("Subsystem Tombstone list", 5, SU_PATH, "root", "ls", "-l", "/data/tombstones/ramdump", NULL);
};
