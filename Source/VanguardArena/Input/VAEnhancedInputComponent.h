#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Input/VAInputConfig.h"
#include "VAEnhancedInputComponent.generated.h"

UCLASS()
class VANGUARDARENA_API UVAEnhancedInputComponent : public UEnhancedInputComponent
{
    GENERATED_BODY()

public:
    template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
    void BindAbilityActions(
        const UVAInputConfig* InputConfig,
        UserClass* Object,
        PressedFuncType PressedFunc,
        ReleasedFuncType ReleasedFunc
    );
};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UVAEnhancedInputComponent::BindAbilityActions(
    const UVAInputConfig* InputConfig,
    UserClass* Object,
    PressedFuncType PressedFunc,
    ReleasedFuncType ReleasedFunc)
{
    check(InputConfig);
    
    for (const FVAInputAction& Action : InputConfig->AbilityActions)
    {
        if (Action.InputAction && Action.InputTag.IsValid())
        {
       
            if (PressedFunc)
            {
                BindAction(
                    Action.InputAction,             
                    ETriggerEvent::Triggered,     
                    Object,                      
                    PressedFunc,                    
                    Action.InputTag               
                );
            }
            
            if (ReleasedFunc)
            {
                BindAction(
                    Action.InputAction,
                    ETriggerEvent::Completed,       
                    Object,
                    ReleasedFunc,
                    Action.InputTag
                );
            }
        }
    }
}