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
    /**
     * Ability input'larını bağla
     * 
     * InputConfig'deki her eşleştirme için:
     * - Pressed callback bağla (aktive et)
     * - Released callback bağla (bırakma / cancel)
     * 
     * Template kullanımı: Herhangi bir sınıfın fonksiyonunu callback olarak bağlayabilmek için.
     * UserObject = genelde Character sınıfı
     * 
     * @param InputConfig: Input-Tag eşleştirme tablosu
     * @param Object: Callback'lerin sahibi (character)
     * @param PressedFunc: Basılınca çağrılacak fonksiyon
     * @param ReleasedFunc: Bırakılınca çağrılacak fonksiyon
     */
    template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
    void BindAbilityActions(
        const UVAInputConfig* InputConfig,
        UserClass* Object,
        PressedFuncType PressedFunc,
        ReleasedFuncType ReleasedFunc
    );
};

// ─── TEMPLATE İMPLEMENTASYON ───
// Template fonksiyonlar header'da implemente edilir (C++ kuralı)
template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UVAEnhancedInputComponent::BindAbilityActions(
    const UVAInputConfig* InputConfig,
    UserClass* Object,
    PressedFuncType PressedFunc,
    ReleasedFuncType ReleasedFunc)
{
    // Null kontrol
    check(InputConfig);

    // Config'deki her eşleştirmeyi döngüle
    for (const FVAInputAction& Action : InputConfig->AbilityActions)
    {
        // Geçerlilik kontrolü
        if (Action.InputAction && Action.InputTag.IsValid())
        {
            // PRESSED (basılınca) binding
            // ETriggerEvent::Triggered = tuş basıldığında
            if (PressedFunc)
            {
                BindAction(
                    Action.InputAction,             // Hangi Input Action?
                    ETriggerEvent::Triggered,       // Ne zaman? Basılınca
                    Object,                         // Kimin fonksiyonu?
                    PressedFunc,                    // Hangi fonksiyon?
                    Action.InputTag                 // Parametre olarak tag gönder
                );
            }

            // RELEASED (bırakılınca) binding
            // ETriggerEvent::Completed = tuş bırakıldığında
            if (ReleasedFunc)
            {
                BindAction(
                    Action.InputAction,
                    ETriggerEvent::Completed,       // Bırakılınca
                    Object,
                    ReleasedFunc,
                    Action.InputTag
                );
            }
        }
    }
}