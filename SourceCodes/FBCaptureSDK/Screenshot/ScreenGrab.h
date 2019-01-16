/*
*  Copyright (c) 2014, Facebook, Inc.
*  All rights reserved.
*
*  This source code is licensed under the BSD-style license found in the
*  LICENSE file in the root directory of this source tree. An additional grant
*  of patent rights can be found in the PATENTS file in the same directory.
*
*/

#pragma once

#include <d3d11_1.h>
#include <ocidl.h>
#include <stdint.h>
#include <functional>
#include "Common/Log.h"

using namespace FBCapture::Common;

namespace FBCapture {
	namespace Screenshot {
		HRESULT SaveDDSTextureToFile(_In_ ID3D11DeviceContext* pContext,
																 _In_ ID3D11Resource* pSource,
																 _In_z_ const wchar_t* fileName);

		HRESULT SaveWICTextureToFile(_In_ ID3D11DeviceContext* pContext,
																 _In_ ID3D11Resource* pSource,
																 _In_ REFGUID guidContainerFormat,
																 _In_z_ const wchar_t* fileName,
																 _In_opt_ const GUID* targetFormat = nullptr,
																 _In_opt_ std::function<void(IPropertyBag2*)> setCustomProps = nullptr,
																 _In_ bool is360 = false);
	}
}
