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
* \file DBHandler.h
* \author Matthias Birnthaler Matthias-Birnthaler@outlook.com
* \date 15 May 2019
* \brief Interface to the database
*
*/

#ifndef DBHANDLER_H
#define DBHANDLER_H

/* ================================================[includes]================================================ */

#include <sqlite_modern_cpp.h>

/*!
* Database Inferface class 
*/
class DBHandler
{
private:
	/*! sqlite database */
    sqlite::database mdb;

public:
	/*!
	* destructor of the DBHandler
	*/
    virtual ~DBHandler() = default;

	/*!
	* constructor of the DBHandler
	*/
    explicit DBHandler(const std::string& path);

	/*!
	* database interfaces; see implementation for further information
	*/
    void AddTribe(const int TribeId);
    std::vector<int> GetTribeSlotsTimer(const int TribeId);
    bool UpdateSlotTimer(const int TribeId, const std::vector<int> SlotTimer);
    bool IsTribeInDatabase(int TribeId);
    void DeleteTribe(int TribeId);
	void WipeDatabase();

};


#endif /* DBHANDLER_H */

/* =================================================[end of file]================================================= */
