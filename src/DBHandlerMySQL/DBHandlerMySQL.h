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
* \file DBHandlerMySQL.h
* \author Matthias Birnthaler Matthias-Birnthaler@outlook.com
* \date 23 November 2020
* \brief Interface to the database for mysql
*
*/

#ifndef DBHANDLERMYSQL_H
#define DBHANDLERMYSQL_H

/* ================================================[includes]================================================ */

#include <mysql++11.h>


/*!
* Database Inferface class 
*/
class DBHandlerMySQL
{
private:
public:
	/*!
	* destructor of the DBHandler
	*/
    virtual ~DBHandlerMySQL() = default;

	/*!
	* constructor of the DBHandler
	*/
    explicit DBHandlerMySQL(std::string server, std::string username, std::string password, std::string db_name);

    /*!
    * sql database
    */
    daotk::mysql::connection db_;


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
