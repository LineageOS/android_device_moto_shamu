/*
 * Copyright (C) 2016 The CyanogenMod Project
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

package com.android.internal.telephony;

import android.content.Context;
import android.os.Message;

public class shamuRIL extends RIL implements CommandsInterface {
    private boolean setPreferredNetworkTypeSeen = false;
    private boolean dataAllowed = false;

    public shamuRIL(Context context, int preferredNetworkType, int cdmaSubscription) {
        this(context, preferredNetworkType, cdmaSubscription, null);
    }

    public shamuRIL(Context context, int preferredNetworkType,
            int cdmaSubscription, Integer instanceId) {
        super(context, preferredNetworkType, cdmaSubscription, instanceId);
    }

    @Override
    public void setPreferredNetworkType(int networkType , Message response) {
        riljLog("setPreferredNetworkType: " + networkType);

        setPreferredNetworkTypeSeen = true;

        super.setPreferredNetworkType(networkType, response);
    }

    @Override
    public void setDataAllowed(boolean allowed, Message result) {
        dataAllowed = allowed;
        mVoiceNetworkStateRegistrants.notifyRegistrants(new AsyncResult(null, null, null));

        super.setDataAllowed(allowed, result);
    }

    @Override
    protected RILRequest
    processSolicited (Parcel p) {
        int serial, error, request;
        RILRequest rr;
        int dataPosition = p.dataPosition(); // save off position within the Parcel

        serial = p.readInt();
        error = p.readInt();

        rr = mRequestList.get(serial);
        if (rr == null || error != 0 || p.dataAvail() <= 0) {
            p.setDataPosition(dataPosition);
            return super.processSolicited(p);
        }

        try { switch (rr.mRequest) {
            case RIL_REQUEST_DATA_REGISTRATION_STATE:
                String dataRegStates[] = (String [])responseStrings(p);

                riljLog("DataRegistrationState response");

                if (dataRegStates.length > 0) {
                    if (dataRegStates[0] != null) {
                        if (!dataAllowed) {
                            if (Integer.parseInt(dataRegStates[0]) > 0) {
                                riljLog("modifying dataRegState to 0 from " + dataRegStates[0]);
                                dataRegStates[0] = "0";
                            }
                        }
                    }
                }

                if (RILJ_LOGD) riljLog(rr.serialString() + "< " + requestToString(rr.mRequest)
                                + " " + retToString(rr.mRequest, dataRegStates));

                if (rr.mResult != null) {
                        AsyncResult.forMessage(rr.mResult, dataRegStates, null);
                        rr.mResult.sendToTarget();
                }
                mRequestList.remove(serial);
                break;
            default:
                p.setDataPosition(dataPosition);
                return super.processSolicited(p);
        }} catch (Throwable tr) {
                // Exceptions here usually mean invalid RIL responses

                Rlog.w(RILJ_LOG_TAG, rr.serialString() + "< "
                                + requestToString(rr.mRequest)
                                + " exception, possible invalid RIL response", tr);

                if (rr.mResult != null) {
                        AsyncResult.forMessage(rr.mResult, null, tr);
                        rr.mResult.sendToTarget();
                }
                return rr;
        }

        return rr;
    }

    @Override
    protected void
    processUnsolicited (Parcel p) {
        int dataPosition = p.dataPosition(); // save off position within the Parcel
        int response;

        response = p.readInt();

        switch(response) {
            case RIL_UNSOL_RIL_CONNECTED:
                if (!setPreferredNetworkTypeSeen) {
                    riljLog("RIL: connected, setting network type to " + mPreferredNetworkType);
                    setPreferredNetworkType(mPreferredNetworkType, null);
                }
                break;
        }

        p.setDataPosition(dataPosition);
        super.processUnsolicited(p);
    }
}
