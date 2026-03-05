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
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify: EventTag boş!"));
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

	UE_LOG(LogTemp, Verbose, TEXT("AnimNotify: Event gönderildi → %s"), *EventTag.ToString());
}

FString UVAAnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		return FString::Printf(TEXT("VA: %s"), *EventTag.ToString());
	}
	return TEXT("VA: Send Event (TAG YOK!)");
}