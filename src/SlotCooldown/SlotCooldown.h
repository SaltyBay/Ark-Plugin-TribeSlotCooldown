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
* \file SlotCooldown.h
* \author Matthias Birnthaler Matthias-Birnthaler@outlook.com
* \date 15 May 2019
* \brief Implementation of the player slots cooldown logic 
*
*/

#ifndef SLOTCOOLDOWN_H
#define SLOTCOOLDOWN_H

/* ================================================[includes]================================================ */
#include <API/Ark/Ark.h>
#include "json.hpp"
#include "DBHandler.h"


namespace SlotCooldown
{
    /* ================================================[declaration of public data]============================== */

	/** \brief Interface database */
    inline std::unique_ptr<DBHandler> database;


	extern FString SuppressPlayerJoinTribeMessage;
	extern FString SuppressMergeTribeMessage;
	extern FString CommandDisplaySlotsMessage;
	extern FString CommandDisplaySlotsMessageSlotCooldown;


	extern FString CommandPrefix;
	extern FString CommandDisplaySlots;

	extern float MessageDisplaySize;
	extern float MessageDisplayTime;

	extern int SlotCooldown;

	extern bool AutoWipeDatabase;

    extern float DelayActivationTime;


    /* ================================================[declaration of public functions]========================= */

    extern void InitSlotCooldown();
    extern void NormalizeSlots(std::vector<int>* slots, long double ServerRunTime);
    extern void SetTribeSlotToCooldown(int TribeId);
    extern bool SuppressPlayerJoinTribe(int TribeId, int PlayersInTribe);
    extern bool SuppressTribeMerge(int TribeIdNewTribe, int TribeIdOldTribe, int NumPlayersInNewTribe, int NumPlayersInOldTribe);

}


#endif /* SLOTCOOLDOWN_H */

/* =================================================[end of file]================================================= */
