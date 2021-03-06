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
* \file SlotCooldown.cpp
* \author Matthias Birnthaler Matthias-Birnthaler@outlook.com
* \date 15 May 2019
* \brief Implementation of the player slots cooldown logic
*
*/


/* =================================================[includes]================================================= */

#include "SlotCooldown.h"
#include <fstream>


namespace SlotCooldown
{
	/* =============================================== [global data] =============================================== */

    /* ========================================== [local defines] =============================================== */
    #define FACTOR_HOURS_TO_SECONDS (int)3600


	/** \brief Message for the player if tribe join is not possible */
    FString SuppressPlayerJoinTribeMessage;
	/** \brief Message for the player if tribe merge is not possible */
    FString SuppressMergeTribeMessage;
	/** \brief Message for the player with the total of tribe player slots with cooldown  */
    FString CommandDisplaySlotsMessage;
	/** \brief Message for the player with the left time for a slots cooldown */
    FString CommandDisplaySlotsMessageSlotCooldown;

	/** \brief Prefix for chat commands */
    FString CommandPrefix;
	/** \brief String for chat command display slots with cooldown */
    FString CommandDisplaySlots;

	/** \brief Size of player notifications */
	float MessageDisplaySize; 
	/** \brief Display time for player notifications */
	float MessageDisplayTime;

	/** \brief Cooldown time for slots in secounds */
	int SlotCooldown;

	/** \brief Wipes database if a new world is detected */
	bool AutoWipeDatabase;

    /** \brief delays the activation of the tribe slot cooldown (in hours) */
    float DelayActivationTime;


	/* ===================================== [prototype of local functions] ======================================= */

	static nlohmann::json ReadConfig(void);
	static bool GetFreeSlot(std::vector<int> SlotsTimer, int ServerRunTime);
	static void MergeTribeCooldowns(int TribId, std::vector<int> * SlotsNewTribe, std::vector<int> * SlotsOldTribe, int ServerRunTime);
	static int CountSlotsWithCooldown(std::vector<int> SlotsTimer, int ServerRunTime);


	/* ===================================== [definition of local functions] ====================================== */

    /**
    * \brief Read Config
    *
    * This function reads out the json configuration file
    *
    * \return nlohmann::json configuration of the plugin
    */
    static nlohmann::json ReadConfig(void)
    {
		nlohmann::json config;

        const std::string config_path = ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/TribeSlotCooldown/config.json";
        std::ifstream file{ config_path };
        if (!file.is_open())
        {
            throw std::runtime_error("Can't open config.json");
        }

        file >> config;

        file.close();

		return config;
    }


	/**
	* \brief Checks if a slot is available
	*
	* This function checks if a slot without cooldown is available
	*
	* \param[in] SlotsTimer vector with all cooldowns of a tribe
	* \param[in] ServerRunTime the current server runntime
	* \return int number of slots with cooldown
	*/
	static bool GetFreeSlot(std::vector<int> SlotsTimer, int ServerRunTime)
	{
		bool result = false;
		int tribelimit = ArkApi::GetApiUtils().GetShooterGameMode()->MaxNumberOfPlayersInTribeField();

		if ((int)(SlotsTimer.size() < tribelimit - 1))
		{
			result = true;
		}
		else
		{
			for (std::vector<int>::iterator it = SlotsTimer.begin(); it != SlotsTimer.end(); ++it)
			{
				if (*it < ServerRunTime)
				{
					SlotsTimer.erase(it);
					result = true;
					break;
				}
			}
		}
		return result;
	}


	/**
	* \brief Merge the slots with cooldowns
	*
	* This function merges the slots with cooldown of two tribes. Expired cooldowns will get removed
	*
	* \param[in] TribId the id of the tribe in which to merge
	* \param[in/out] SlotsNewTribe slots of the new tribe
	* \param[in] SlotsOldTribe slots of the old tribe
	* \param[in] ServerRunTime the current server runntime
	* \return void
	*/
	static void MergeTribeCooldowns(int TribId, std::vector<int> * SlotsNewTribe, std::vector<int> * SlotsOldTribe, int ServerRunTime)
	{
		int tribelimit = ArkApi::GetApiUtils().GetShooterGameMode()->MaxNumberOfPlayersInTribeField();

		if ((nullptr != SlotsOldTribe) && (nullptr != SlotsNewTribe))
		{
			for (std::vector<int>::iterator it = SlotsOldTribe->begin(); it != SlotsOldTribe->end(); ++it)
			{
				if (*it > ServerRunTime)
				{
					SlotsNewTribe->push_back(*it);
				}
			}
			NormalizeSlots(SlotsNewTribe, ServerRunTime);

			if (SlotsNewTribe->size() >= tribelimit)
			{
				Log::GetLog()->error("data inconsistency: Tribe {} has {} slots on cooldown !", TribId, SlotsNewTribe->size());
			}
		}
	}


    /**
    * \brief Counts the slots with cooldown
    *
    * This function counts all slots of a tribe that are not expired
    *
    * \param[in] SlotsTimer vector with all cooldowns of a tribe
    * \param[in] ServerRunTime the current server runntime
    * \return int number of slots with cooldown 
    */
    static int CountSlotsWithCooldown(std::vector<int> SlotsTimer, int ServerRunTime)
    {
        int count = 0; 

        for (std::vector<int>::iterator it = SlotsTimer.begin(); it != SlotsTimer.end(); ++it)
        {
            if (*it > ServerRunTime)
            {
                count++;
            }
        }

        return count;
    }


	/* ===================================== [definition of global functions] ===================================== */

    /**
    * \brief Initialisation of the Slot Cooldown
    *
    * This function opens the database connection and initializes some variables  
    *
    * \return void
    */
    void InitSlotCooldown(void)
    {
		nlohmann::json config = ReadConfig();

        const std::string db_path = config["General"]["DbPathOverride"];
        
        database = std::make_unique<DBHandler>(db_path);

        SuppressPlayerJoinTribeMessage = FString(ArkApi::Tools::Utf8Decode(config["Messages"]["SuppressPlayerJoinTribeMessage"]).c_str());
        SuppressMergeTribeMessage = FString(ArkApi::Tools::Utf8Decode(config["Messages"]["SuppressMergeTribeMessage"]).c_str());

        CommandDisplaySlotsMessage = FString(ArkApi::Tools::Utf8Decode(config["Messages"]["CommandDisplaySlotsMessage"]).c_str());
        CommandDisplaySlotsMessageSlotCooldown = FString(ArkApi::Tools::Utf8Decode(config["Messages"]["CommandDisplaySlotsMessageSlotCooldown"]).c_str());

        CommandPrefix = FString(ArkApi::Tools::Utf8Decode(config["Commands"]["CommandPrefix"]).c_str());
        CommandDisplaySlots = FString(ArkApi::Tools::Utf8Decode(config["Commands"]["CommandDisplaySlots"]).c_str());
    
		MessageDisplaySize = (float)(config["General"].value("MessageTextSize", 1.4));
		MessageDisplayTime = (float)(config["General"].value("MessageDisplayDelay", 10));

		auto slotcooldown = config["General"].value("SlotCooldown", 24.0);
		SlotCooldown = (int)slotcooldown * FACTOR_HOURS_TO_SECONDS;

		AutoWipeDatabase = config["General"].value("AutoWipeDatabase", false);

        auto delayActivationTime = config["General"].value("DelayActivationTime", 0.0);

        DelayActivationTime = (int)delayActivationTime * FACTOR_HOURS_TO_SECONDS;
    }


    /**
    * \brief Normalize slot cooldowns
    *
    * This function is to normalize the slot cooldown data. Expired cooldowns will get removed. 
    * Expired times will get sort ascending.
    *
    * \param[in] slots vector with slots to normalize
    * \param[in] ServerRunTime the current server runntime
    * \return void
    */
    void NormalizeSlots(std::vector<int>* slots, long double ServerRunTime)
    {
        if (nullptr != slots)
        {
            for (std::vector<int>::iterator it = slots->begin(); it != slots->end();)
            {
                if (*it < ServerRunTime)
                {
					it = slots->erase(it);
                }
				else
				{
					++it;
				}
            }

			if (false == slots->empty())
			{
				std::sort(slots->begin(), slots->end());
			}
        }
    }


    /**
    * \brief Sets tribe slot to cooldown
    *
    * This function sets one of a free slot of a given tribe to cooldown
    *
    * \param[in] TribeId the id of the tribe for which a slot will set on cooldown
    * \return void
    */
    void SetTribeSlotToCooldown(int TribeId)
    {
        std::vector<int> slots;
        long double currentServerTime;

        /* add tribe to database if not exist */
        if (false == database->IsTribeInDatabase(TribeId))
        {
            database->AddTribe(TribeId);
        }

        /* select tribe slot cooldowns from databse*/
        slots = database->GetTribeSlotsTimer(TribeId);

        /* update tribe slot cooldowns */
        currentServerTime = ArkApi::GetApiUtils().GetWorld()->TimeSecondsField();
        int slotBlockedUntil = (int)currentServerTime + SlotCooldown;

        if (true == GetFreeSlot(slots, (int)currentServerTime))
        {
            slots.push_back(slotBlockedUntil);
        }


        /* rewrite  tribe slots to database */
        database->UpdateSlotTimer(TribeId, slots);
    }


    /**
    * \brief Checks if it is possible to join a tribe
    *
    * This function checks if a free slot is available to join a tribe
    *
    * \param[in] TribeId the id of tribe id to check
    * \param[in] PlayersInTribe number of players in the tribe
    * \return true if tribe join is not possible, otherwise false
    */
    bool SuppressPlayerJoinTribe(int TribeId, int PlayersInTribe)
    {
        bool result = true;
        int NumOfSlotsWithCooldownTribe = 0;
        long double currentServerTime = ArkApi::GetApiUtils().GetWorld()->TimeSecondsField();
        int tribelimit = ArkApi::GetApiUtils().GetShooterGameMode()->MaxNumberOfPlayersInTribeField();
        std::vector<int> slots;


        if (true == database->IsTribeInDatabase(TribeId))
        {
            slots = database->GetTribeSlotsTimer(TribeId);
            NumOfSlotsWithCooldownTribe = CountSlotsWithCooldown(slots, (int)currentServerTime);
        
            if (tribelimit > (PlayersInTribe + NumOfSlotsWithCooldownTribe))
            {
                result = false;
            }
        
        }
        else /* never removed a player from tribe */
        {
            result = false;
        }

        return result;
    }


    /**
    * \brief Checks if it is possible to merge a tribe
    *
    * This function checks if there are enoth free spots to perform a tribe merge. 
    * Slots on cooldown of the old tribe will be inherited to the new tribe
    *
    * \param[in] TribeIdNewTribe the id of tribe in witch to merge 
    * \param[in] TribeIdOldTribe the id of the old tribe
    * \param[in] NumPlayersInNewTribe the current number of player in the new tribe 
    * \param[in] NumPlayersInOldTribe the current number of player in the old tribe
    * \return true if tribe merge is not possible, otherwise false
    */
    bool SuppressTribeMerge(int TribeIdNewTribe, int TribeIdOldTribe, int NumPlayersInNewTribe, int NumPlayersInOldTribe)
    {
        bool result = true;
        bool isNewTribeInDb;
        bool isOldTribeInDb;
        int NumOfNeededTribeSlots = 0;
        int NumOfSlotsWithCooldownOldTribe = 0;
        int NumOfSlotsWithCooldownNewTribe = 0;
        std::vector<int> slotsOfNewTribe;
        std::vector<int> slotsOfOldTribe;
        long double currentServerTime = ArkApi::GetApiUtils().GetWorld()->TimeSecondsField();
        int tribelimit = ArkApi::GetApiUtils().GetShooterGameMode()->MaxNumberOfPlayersInTribeField();

        /* check if the old tribe has tribe slots on cooldown */
        isOldTribeInDb = database->IsTribeInDatabase(TribeIdOldTribe);
        if (true == isOldTribeInDb)
        {
            slotsOfOldTribe = database->GetTribeSlotsTimer(TribeIdOldTribe);
            NumOfSlotsWithCooldownOldTribe = CountSlotsWithCooldown(slotsOfOldTribe, (int)currentServerTime);
        }

        /* check if the new tribe has tribe slots on cooldown */
        isNewTribeInDb = database->IsTribeInDatabase(TribeIdNewTribe);
        if (true == isNewTribeInDb)
        {
            slotsOfNewTribe = database->GetTribeSlotsTimer(TribeIdNewTribe);
            NumOfSlotsWithCooldownNewTribe = CountSlotsWithCooldown(slotsOfNewTribe, (int)currentServerTime);
        }

        /* the total of slots on cooldown and tribemember must not exceed the tribe limit */
        if (tribelimit >= (NumPlayersInOldTribe + NumOfSlotsWithCooldownNewTribe +
            NumPlayersInNewTribe + NumOfSlotsWithCooldownOldTribe))
        {
            /* merge is possible inherit the slot cooldowns of the old tribe to the new one */
            MergeTribeCooldowns(TribeIdNewTribe, &slotsOfNewTribe, &slotsOfOldTribe, (int)currentServerTime);
        
            /* save slots of the new tribe in the database */
            if (false == isNewTribeInDb)
            {
                database->AddTribe(TribeIdNewTribe);
            }
            database->UpdateSlotTimer(TribeIdNewTribe, slotsOfNewTribe);

            /* delete the database entry of the old tribe */
            if (true == isOldTribeInDb)
            {
                database->DeleteTribe(TribeIdOldTribe);
            }

            result = false;
        }
        return result;
    }

}

/* =================================================[end of file]================================================= */
