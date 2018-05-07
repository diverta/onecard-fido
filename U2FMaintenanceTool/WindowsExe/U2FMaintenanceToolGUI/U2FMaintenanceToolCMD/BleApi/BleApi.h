/*
 *   Copyright (C) 2016, VASCO Data Security Int.
 *   Author: Johan.Verrept@vasco.com
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BLEAPI_BLEAPI_H_
#define _BLEAPI_BLEAPI_H_

#include "BleApiTypes.h"
#include "BleDevice.h"

#include <string>
#include <vector>

typedef class BleApi {
protected:
  BleApi(BleApiConfiguration &configuration);
  ~BleApi(void);

public:
  virtual std::vector < pBleDevice > findDevices();
  virtual bool IsEnabled();

  static BleApi *CreateAPI(BleApiConfiguration &configuration);

  U2FVersion GetU2FVersion() { return mConfiguration.version; };

 protected:
   BleApiConfiguration        mConfiguration;
	 std::vector < pBleDevice > mDeviceList;
} *pBleApi;

#endif				/* _BLEAPI_BLEAPI_H_ */
