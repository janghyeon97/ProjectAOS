// Fill out your copyright notice in the Description page of Project Settings.


#include "Plugins/MultiplaySessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Kismet/GameplayStatics.h"
#include "Structs/SessionInfomation.h"
#include "Interfaces/OnlineIdentityInterface.h"


UMultiplaySessionSubsystem::UMultiplaySessionSubsystem()
	:CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionCompleted))
	, UpdateSessionCompleteDelegate(FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnUpdateSessionCompleted))
	, StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionCompleted))
	, EndSessionCompleteDelegate(FOnEndSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnEndSessionCompleted))
	, DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionCompleted))
	, FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsCompleted))
	, JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionCompleted))
{
}

void UMultiplaySessionSubsystem::CreateSession(bool IsDedicated, bool IsLANMatch, FString SessionName, int32 NumPublicConnections, FString MatchMap, FString Password)
{
	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem)
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (SessionInterface)
	{
		SessionSettings = MakeShareable(new FOnlineSessionSettings());
		SessionSettings->bIsDedicated = false;
		SessionSettings->bAllowInvites = true;
		SessionSettings->bIsLANMatch = IsLANMatch;
		SessionSettings->NumPublicConnections = NumPublicConnections;
		SessionSettings->bUseLobbiesIfAvailable = false;
		SessionSettings->bUsesPresence = false;
		SessionSettings->bShouldAdvertise = true;
		SessionSettings->bAllowJoinInProgress = true;
		SessionSettings->bAllowJoinViaPresence = true;

		Password.RemoveSpacesInline();

		UE_LOG(LogTemp, Warning, TEXT("UMultiplaySessionSubsystem - Password :: %s"), *Password);
		bool IsExistPassword = Password.Len() > 0 ? true : false;

		SessionSettings->Set(SEARCH_KEYWORDS, FString("ProjectAOS"), EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(FName("SessionName"), SessionName, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(FName("MapName"), MatchMap, EOnlineDataAdvertisementType::ViaOnlineService);
		SessionSettings->Set(FName("IsExistPassword"), IsExistPassword, EOnlineDataAdvertisementType::ViaOnlineService);
		if(IsExistPassword) SessionSettings->Set(FName("Password"), Password, EOnlineDataAdvertisementType::ViaOnlineService);

		CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

		if (!SessionInterface->CreateSession(0, FName("MainSession"), *SessionSettings))
		{
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
			OnCreateSessionCompleteEvent.Broadcast(false);
		}
	}
}

void UMultiplaySessionSubsystem::OnCreateSessionCompleted(FName SessionName, bool Successful)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	if (Successful)
	{
		if (OnCreateSessionCompleteEvent.IsBound())
		{
			OnCreateSessionCompleteEvent.Broadcast(Successful);
		}
		
		TryServerTravel(L"/Game/ProjectAOS/Level/Lobby?Listen");
	}
}

void UMultiplaySessionSubsystem::FindSessions(int32 MaxSearchResults, bool IsLANQuery)
{
	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem)
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<USessionInfomation*>(), false);
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<USessionInfomation*>(), false);
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	SessionSearch->MaxSearchResults = 20;
	
	if (!SessionInterface->FindSessions(0, SessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		OnFindSessionsCompleteEvent.Broadcast(TArray<USessionInfomation*>(), false);
	}
}

void UMultiplaySessionSubsystem::OnFindSessionsCompleted(bool Successful)
{
	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem)
	{
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		return;
	}

	if (SessionSearch->SearchResults.Num() <= 0)
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<USessionInfomation*>(), false);
		return;
	}

	int index = 0;
	bool IsExistPassword;
	FString SessionName;
	FString Password;
	FString MapName;

	TArray<USessionInfomation*> SessionSearchResults;
	for (auto& Result : SessionSearch->SearchResults)
	{
		USessionInfomation* SessionInfo = NewObject<USessionInfomation>(this, USessionInfomation::StaticClass());
		Result.Session.SessionSettings.Get(FName("IsExistPassword"), IsExistPassword);
		Result.Session.SessionSettings.Get(FName("SessionName"), SessionName);
		Result.Session.SessionSettings.Get(FName("Password"), Password);
		Result.Session.SessionSettings.Get(FName("MapName"), MapName);


		SessionInfo->SetIndex(index);
		SessionInfo->SetSessionName(SessionName);
		SessionInfo->SetMapName(MapName);
		SessionInfo->SetMaxPlayers(Result.Session.SessionSettings.NumPublicConnections);
		SessionInfo->SetCurrentPlayers(Result.Session.SessionSettings.NumPublicConnections - Result.Session.NumOpenPublicConnections);
		SessionInfo->SetPing(Result.PingInMs);
		SessionInfo->SetIsExistPassword(IsExistPassword);
		//SessionInfo->SetSessionResult(Result);

		SessionSearchResults.Add(SessionInfo);
		index++;
	}

	OnFindSessionsCompleteEvent.Broadcast(SessionSearchResults, true);
}

void UMultiplaySessionSubsystem::JoinSession(int Index, FString InputPassword)
{
	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem)
	{
		OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	bool IsExistPassword;
	if (SessionSearch->SearchResults.IsValidIndex(Index))
	{
		SessionSearch->SearchResults[Index].Session.SessionSettings.Get(FName("IsExistPassword"), IsExistPassword);
		if (IsExistPassword)
		{
			FString Password;
			InputPassword.RemoveSpacesInline();
			SessionSearch->SearchResults[Index].Session.SessionSettings.Get(FName("Password"), Password);
			if (InputPassword.Equals(Password))
			{
				JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

				if (!SessionInterface->JoinSession(0, FName("MainSession"), SessionSearch->SearchResults[Index]))
				{
					SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
					OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("JoinSession - Password is not matched!!!"));
				SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
				OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
			}
		}
		else
		{
			JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

			if (!SessionInterface->JoinSession(0, FName("MainSession"), SessionSearch->SearchResults[Index]))
			{
				SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
				OnJoinSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
			}
		}
	}
}

void UMultiplaySessionSubsystem::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{

}

void UMultiplaySessionSubsystem::UpdateSession(FString MapName)
{

}

void UMultiplaySessionSubsystem::OnUpdateSessionCompleted(FName SessionName, bool Successful)
{

}

void UMultiplaySessionSubsystem::StartSession()
{

}

void UMultiplaySessionSubsystem::OnStartSessionCompleted(FName SessionName, bool Successful)
{

}

void UMultiplaySessionSubsystem::EndSession()
{

}

void UMultiplaySessionSubsystem::OnEndSessionCompleted(FName SessionName, bool Successful)
{

}

void UMultiplaySessionSubsystem::DestroySession()
{
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem)
	{
		OnDestroySessionCompleteEvent.Broadcast(false);
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		OnDestroySessionCompleteEvent.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(FName("MainSession")))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		OnDestroySessionCompleteEvent.Broadcast(false);
	}
}

void UMultiplaySessionSubsystem::OnDestroySessionCompleted(FName SessionName, bool Successful)
{

}

bool UMultiplaySessionSubsystem::TryServerTravel(FString OpenLevelName)
{
	GetWorld()->ServerTravel(OpenLevelName);
	return true;
}

bool UMultiplaySessionSubsystem::TryClientTravelToCurrentSession(FString OpenLevelName)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		return false;
	}

	FString connectString;
	if (!sessionInterface->GetResolvedConnectString(NAME_GameSession, connectString))
	{
		return false;
	}

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	playerController->ClientTravel(connectString, TRAVEL_Absolute);
	return true;
}

void UMultiplaySessionSubsystem::LoginWithEOS(const FString& ID, const FString& Token, const FString& LoginType)
{
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (OnlineSubsystem)
	{
		IOnlineIdentityPtr IdentityPointerRef = OnlineSubsystem->GetIdentityInterface();
		if (IdentityPointerRef)
		{
			FOnlineAccountCredentials AccountDetails;
			AccountDetails.Id = ID;
			AccountDetails.Token = Token;
			AccountDetails.Type = LoginType;

			IdentityPointerRef->OnLoginCompleteDelegates->AddUObject(this, &UMultiplaySessionSubsystem::OnLoginCompleted);
			IdentityPointerRef->Login(0, AccountDetails);
		}
	}
}

FString UMultiplaySessionSubsystem::GetPlayerName()
{
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (OnlineSubsystem)
	{
		IOnlineIdentityPtr IdentityPointerRef = OnlineSubsystem->GetIdentityInterface();
		if (IdentityPointerRef)
		{
			if (IdentityPointerRef->GetLoginStatus(0) == ELoginStatus::LoggedIn)
			{
				return IdentityPointerRef->GetPlayerNickname(0);
			}
		}
	}
	return FString();
}

bool UMultiplaySessionSubsystem::IsPlayerLoggedIn() const
{
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (OnlineSubsystem)
	{
		IOnlineIdentityPtr IdentityPointerRef = OnlineSubsystem->GetIdentityInterface();
		if (IdentityPointerRef)
		{
			if (IdentityPointerRef->GetLoginStatus(0) == ELoginStatus::LoggedIn)
			{
				return true;
			}
		}
	}
	return false;
}

void UMultiplaySessionSubsystem::OnLoginCompleted(int32 LocalUserNum, bool bSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Login Success"));
		OnLoginCompleteEvent.Broadcast(true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Login Fail :: %s"), *Error);
		OnLoginCompleteEvent.Broadcast(false);
	}
}

void UMultiplaySessionSubsystem::CreateEOSSession(bool bIsDedicatedServer, bool bIsLanServer, int32 NumberOfPublicConnections)
{

}
