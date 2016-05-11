/*
  JSONStreamingParser.h - JSON parser library for low resources controllers
  Copyright (c) 2015 Massimo Del Fedele.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef _JsonStreamingParser_h_
#define _JsonStreamingParser_h_

#include <stdint.h>
#include <stdlib.h>

// callback on match
typedef void (*JSONCallback)(uint8_t filter, uint8_t level, const char *name, const char *value, void *cbObj);

// minimum name and value sizes (gets incremented up to maximum)
#define JSON_MIN_DATA_LEN			10
#define JSON_DATA_LEN_INCREMENT		10

class JSONStreamingParser
{
	private:
	
		// parser state
		uint8_t state;
		
		// parser flags definitions
		enum
		{
			IN_QUOTES			= 0x01,
			IN_DQUOTES			= 0x02,
			IN_BACKSLASH		= 0x04,
		} PARSER_FLAGS;
		
		// parser flags
		uint8_t flags;
		
		// parsing depth level
		uint8_t level;
		
		// level flags : bit 1 means array, bit 0 means list
		// used to keep track of nested elements
		// here we allow max 32 nesting levels, NOT checked
		uint32_t levels;
		
		// callback on data got from stream
		JSONCallback callback;
		
		// object parameter passed to callback, if needed
		void *callbackObject;
		
		// limits on names and values length
		uint16_t maxDataLen;
		
		// current name and value fields
		char *name;
		uint16_t nameAlloc;
		
		char *value;
		uint16_t valueAlloc;
		
	protected:
	
		// append to name field
		void appendName(char c);
		
		// append to value field
		void appendValue(char c);
		
		// clear name and value fields
		void clearName(void);
		void clearValue(void);
		
		// execute callback and clear name/value
		void doCallback(const char *val = 0);
	
	public:
	
		// parser states definitions
		enum
		{
			PARSER_IDLE				= 0x00,
			PARSER_WAIT_SEPARATOR,
			PARSER_WAIT_NAME,
			PARSER_NAME,
			PARSER_WAIT_SEMICOLON,
			PARSER_WAIT_VALUE,
			PARSER_VALUE,
			PARSER_FINISHED,
			PARSER_ERROR			= 0xff
		} PARSER_STATE;
		
		// constructors
		JSONStreamingParser();
		
		// set the callback
		JSONStreamingParser &setCallback(JSONCallback cb, void *cbObj);
		
		// reset the parser
		JSONStreamingParser &reset(void);
		
		// set the maximum allowed data length
		JSONStreamingParser &setMaxDataLen(uint16_t len = -1);
		
		// feed the parser with a char
		// return value:
		// 1	if needs more chars
		// 0	if finished
		// -1	if error
		uint8_t feed(char c);
		
		// get parser state
		uint8_t getState(void);
		
		// check if finished
		bool isFinished(void);
		
		// check if error
		bool isError(void);
};

#endif
