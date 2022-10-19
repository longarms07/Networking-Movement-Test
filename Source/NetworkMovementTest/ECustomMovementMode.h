#pragma once

#include "UObject/ObjectMacros.h"

/** Custom movement modes for Characters. */
UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_ZipLining   UMETA(DisplayName = "ZipLining"),
	CMOVE_MAX			UMETA(Hidden),
};
