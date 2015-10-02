/*
 * Copyright (C) 2013 The Android Open Source Project
 * Copyright (C) 2015 The CyanogenMod Project
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

#include <healthd.h>

static int mapChargeRateString(const char *charge_rate)
{
    if (strcmp(charge_rate, "Turbo") == 0)
        return android::BATTERY_CHARGE_RATE_FAST_CHARGING;
    else
        return android::BATTERY_CHARGE_RATE_UNKNOWN;
}

void healthd_board_init(struct healthd_config *config)
{
    config->batteryChargeRatePath  = "/sys/class/power_supply/battery/charge_rate";
    config->mapChargeRateString    = mapChargeRateString;
}

int healthd_board_battery_update(__attribute__((unused)) struct android::BatteryProperties *props)
{
    // return 0 to log periodic polled battery status to kernel log
    return 0;
}

