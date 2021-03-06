/*****************************************************************************************************************
* Copyright (C) 2019 by Matthias Birnthaler                                                                      *
*                                                                                                                *
* This file is part of the TribeSlotCooldown Plugin for Ark Server API                                           *
*                                                                                                                *
*    This program is free software : you can redistribute it and/or modify                                       *
*    it under the terms of the GNU General Public License as published by                                        *
*    the Free Software Foundation, either version 3 of the License, or                                           *
*    (at your option) any later version.                                                                         *
*                                                                                                                *
*    This program is distributed in the hope that it will be useful,                                             *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of                                              *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the                                                 *
*    GNU General Public License for more details.                                                                *
*                                                                                                                *
*    You should have received a copy of the GNU General Public License                                           *
*    along with this program.If not, see <https://www.gnu.org/licenses/>.                                        *
*                                                                                                                *
*****************************************************************************************************************/

/**
* \file Hooks.cpp
* \author Matthias Birnthaler Matthias-Birnthaler@outlook.com
* \date 15 May 2019
* \brief File containing the implementation for all needed Hooks
*
*/


/* ================================================[includes]================================================ */
#include "Hooks.h"
#include "SlotCooldown.h"

/* ========================================== [local defines] =============================================== */

#define VERY_SMALL_RUNNTIME 10.0


/* ===================================== [prototype of local functions] ======================================= */

static bool  Hook_AShooterPlayerState_AddToTribe(AShooterPlayerState* _this, FTribeData* MyNewTribe, bool bMergeTribe, bool bForce, bool bIsFromInvite, APlayerController* InviterPC);
static void  Hook_AShooterGameMode_RemovePlayerFromTribe(AShooterGameMode* _this, unsigned __int64 TribeID, unsigned __int64 PlayerDataID, bool bDontUpdatePlayerState);
static void  Hook_AShooterGameMode_BeginPlay(AShooterGameMode* _this);



/* ===================================== [definition of local functions] ====================================== */

/**
* \brief Hook of AddToTribe Implementation
*
* This function hooks the AddToTribe Implementation. If a tribe slot without cooldown is available the original AddToTribe is called.
* In case of a tribe merge slots with cooldowns will be inherited to the new tribe. The total of all members and all slots with cooldown
* must not be larger than the tribelimit 
*
* \param[in] _this AShooterPlayerState player who accepted the tribe invitation
* \param[in] MyNewTribe the tribe data of the inviters
* \param[in] bMergeTribe true if it is a tribe merge otherwise false
* \param[in] bForce, variable is not used
* \param[in] bIsFromInvite, variable is not used
* \param[in] APlayerController, variable is not used
* \return void
*/
static bool  Hook_AShooterPlayerState_AddToTribe(AShooterPlayerState* _this, FTribeData* MyNewTribe, bool bMergeTribe, bool bForce, bool bIsFromInvite, APlayerController* InviterPC)
{
    bool SuppressAdToTribe = false;

    if ((nullptr != MyNewTribe) && (nullptr != _this))
    {
        AShooterPlayerController* player = _this->GetShooterController();
        
        if (nullptr != player)
        {
            if (false == player->bIsAdmin().Get())
            {

                if (false == bMergeTribe) /* only on player joins tribe */
                {
                    auto tribeid = MyNewTribe->TribeIDField();
                    auto numPlayersInTribe = (MyNewTribe->MembersPlayerDataIDField()).Num();

                    SuppressAdToTribe = SlotCooldown::SuppressPlayerJoinTribe(tribeid, numPlayersInTribe);

                    /* notification for the player */
                    if (true == SuppressAdToTribe)
                    {                    
                        ArkApi::GetApiUtils().SendNotification(player, FColorList::Red, SlotCooldown::MessageDisplaySize, 
							SlotCooldown::MessageDisplayTime, nullptr, *SlotCooldown::SuppressPlayerJoinTribeMessage);
                    }


                }
                else /* tribe merge */
                {
                    auto numPlayersInOldTribe = (_this->MyTribeDataField()->MembersPlayerDataIDField()).Num();
                    auto numPlayersInNewTribe = (MyNewTribe->MembersPlayerDataIDField()).Num();

                    SuppressAdToTribe = SlotCooldown::SuppressTribeMerge(MyNewTribe->TribeIDField(), player->TargetingTeamField(),
                        numPlayersInNewTribe, numPlayersInOldTribe);

                    /* notification for the player */
                    if (true == SuppressAdToTribe)
                    {
                        ArkApi::GetApiUtils().SendNotification(player, FColorList::Red, SlotCooldown::MessageDisplaySize, 
							SlotCooldown::MessageDisplayTime, nullptr, *SlotCooldown::SuppressMergeTribeMessage);
                    }
                }
            }
        }
    }

    if (true == SuppressAdToTribe)
    {
        /* the original add to tribe function call is suppressed*/
        return false;
    }
    else 
    {
        /* call the original add to tribe function */
        return AShooterPlayerState_AddToTribe_original(_this, MyNewTribe, bMergeTribe, bForce, bIsFromInvite, InviterPC);
    }
}



/**
* \brief Hook_AShooterGameMode_RemovePlayerFromTribe
*
* This function hooks the RemovePlayerFromTribe Implementation. If the removed player is no server admin a Player slot from
* the tribe is set on cooldown 
*
* \param[in] _this AShooterGameMode, variable is not used 
* \param[in] TribeID the id of the tribe where a player gets removed
* \param[in] PlayerDataID the id of the removed player 
* \param[in] bDontUpdatePlayerState, variable is not used 
* \return void
*/
static void  Hook_AShooterGameMode_RemovePlayerFromTribe(AShooterGameMode* _this, unsigned __int64 TribeID, unsigned __int64 PlayerDataID, bool bDontUpdatePlayerState)
{

    const uint64 steam64 = ArkApi::GetApiUtils().GetSteamIDForPlayerID(PlayerDataID);
    AShooterPlayerController* player = ArkApi::GetApiUtils().FindPlayerFromSteamId(steam64);

    auto currentServerTime = ArkApi::GetApiUtils().GetWorld()->TimeSecondsField();


    if (currentServerTime > SlotCooldown::DelayActivationTime)
    {
        if (nullptr != player)
        {
            /* do not set the slot on cooldown if a server admin left the tribe */
            if (false == player->bIsAdmin().Get())
            {
                SlotCooldown::SetTribeSlotToCooldown(TribeID);
            }
        }
        else /* player is offline */
        {
            SlotCooldown::SetTribeSlotToCooldown(TribeID);
        }
    }

    AShooterGameMode_RemovePlayerFromTribe_original(_this, TribeID, PlayerDataID, bDontUpdatePlayerState);
}


/**
* \brief Hook_AShooterGameMode_BeginPlay
*
* This function hooks the ShooterGameMode BeginPlay Implementation. If configured the plugin checks for a new world 
* and wipes the plugin databse if it is so
*
* \param[in] _this AShooterGameMode, variable is not used
* \return void
*/
static void  Hook_AShooterGameMode_BeginPlay(AShooterGameMode* _this)
{
	long double currentServerTime = ArkApi::GetApiUtils().GetWorld()->TimeSecondsField();

	if ((true == SlotCooldown::AutoWipeDatabase) && (currentServerTime < VERY_SMALL_RUNNTIME))
	{
		SlotCooldown::database->WipeDatabase();
		Log::GetLog()->info("Wiped plugin databse!");
	}

	AShooterGameMode_BeginPlay_original(_this);
}



/* ===================================== [definition of global functions] ===================================== */

/**
* \brief Initialisation of needed Hooks
*
* This function initialise all needed Hooks
*
* \return void
*/
void InitHooks(void)
{
	ArkApi::GetHooks().SetHook("AShooterPlayerState.AddToTribe", &Hook_AShooterPlayerState_AddToTribe,
		&AShooterPlayerState_AddToTribe_original);

	ArkApi::GetHooks().SetHook("AShooterGameMode.RemovePlayerFromTribe",
		&Hook_AShooterGameMode_RemovePlayerFromTribe,
		&AShooterGameMode_RemovePlayerFromTribe_original);

	ArkApi::GetHooks().SetHook("AShooterGameMode.BeginPlay",
		&Hook_AShooterGameMode_BeginPlay, &AShooterGameMode_BeginPlay_original);

}


/**
* \brief Cancellation of needed Hooks
*
* This function removes all needed Hooks.
*
* \return void
*/
void RemoveHooks(void)
{
	ArkApi::GetHooks().DisableHook("AShooterPlayerState.AddToTribe",
		&Hook_AShooterPlayerState_AddToTribe);

	ArkApi::GetHooks().DisableHook("AShooterGameMode.RemovePlayerFromTribe",
		&Hook_AShooterGameMode_RemovePlayerFromTribe);

	ArkApi::GetHooks().DisableHook("AShooterGameMode.BeginPlay", 
		&Hook_AShooterGameMode_BeginPlay);

}

/* =================================================[end of file]================================================= */
