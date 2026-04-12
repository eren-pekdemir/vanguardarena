// VAAnimNotify_SendGameplayEvent.cpp
#include "Animation/VAAnimNotify_SendGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UVAAnimNotify_SendGameplayEvent::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner) return;
	
	if (!EventTag.IsValid())

	{
		return;
	}
	
    
	FGameplayEventData Payload;
	Payload.Instigator = Owner;
	Payload.Target = Owner;
	Payload.EventTag = EventTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,     
		EventTag,  
		Payload     
	);

}

FString UVAAnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		return FString::Printf(TEXT("VA: %s"), *EventTag.ToString());
	}
	return TEXT("VA: Send Event (TAG YOK!)");
}