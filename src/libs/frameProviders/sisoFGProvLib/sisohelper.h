#pragma once

//#define ENABLE_FGRAB

#ifdef ENABLE_FGRAB
#include <fgrab_struct.h>
#include <fgrab_prototyp.h>
#include <fgrab_define.h>
#include <SisoDisplay.h>
#endif

static int getNoOfBitsFromImageFormat(const int format)
{
	int Bits = 8;
#ifdef ENABLE_FGRAB
	switch (format) {
	case FG_GRAY:
		Bits = 8;
		break;
	case FG_GRAY16:
		Bits = 16;
		break;
	case FG_COL24:
	case FG_COL48:
		Bits = 24;
		break;
	default:
		Bits = 8;
		break;
	};
#endif
	return Bits;
}



static int getBoardsCounter()
{
	int boardCounter = 0;
#ifdef ENABLE_FGRAB
	char buffer[256];
	unsigned int buflen = 256;
	buffer[0] = 0;

	// availability : starting with RT 5.2
	if (Fg_getSystemInformation(NULL, INFO_NR_OF_BOARDS, PROP_ID_VALUE, 0, buffer, &buflen) == FG_OK) {
		boardCounter = atoi(buffer);
	}
#endif
	return boardCounter;

}

static std::vector<std::string> detectSiSoBoard()
{
	std::vector<std::string> retVal;
#ifdef ENABLE_FGRAB

	auto i = 0;

	const auto maxNrOfboards = 10;// use a constant no. of boards to query, when evaluations versions minor to RT 5.2
	auto nrOfBoardsFound = 0;
	const auto nrOfBoardsPresent = getBoardsCounter();

	for (i = 0; i < maxNrOfboards; i++) 
	{
		std::string boardName;
		auto skipIndex = false;
		const auto boardType = Fg_getBoardType(i);
		switch (boardType) {
		case PN_MICROENABLE3I:
			boardName = "MicroEnable III";
			break;
		case PN_MICROENABLE3IXXL:
			boardName = "MicroEnable III XXL";
			break;
		case PN_MICROENABLE4AS1CL:
			boardName = "MicroEnable IV AS1-CL";
			break;
		case PN_MICROENABLE4AD1CL:
			boardName = "MicroEnable IV AD1-CL";
			break;
		case PN_MICROENABLE4VD1CL:
			boardName = "MicroEnable IV VD1-CL";
			break;
		case PN_MICROENABLE4AD4CL:
			boardName = "MicroEnable IV AD4-CL";
			break;
		case PN_MICROENABLE4VD4CL:
			boardName = "MicroEnable IV VD4-CL";
			break;
		case PN_MICROENABLE4AQ4GE:
			boardName = "MicroEnable IV AQ4-GE";
			break;
		case PN_MICROENABLE4VQ4GE:
			boardName = "MicroEnable IV VQ4-GE";
			break;
		case PN_MICROENABLE5AQ8CXP4:
			boardName = "MicroEnable V AQ8-CXP";
			break;
		case PN_MICROENABLE5VQ8CXP4:
			boardName = "MicroEnable V VQ8-CXP";
			break;
		case PN_MICROENABLE5VD8CL:
			boardName = "MicroEnable 5 VD8-CL";
			break;
		case PN_MICROENABLE5AD8CL:
			boardName = "MicroEnable 5 AD8-CL";
			break;
		case PN_MICROENABLE5AQ8CXP6D:
			boardName = "MicroEnable 5 AQ8-CXP6D";
			break;
		case PN_MICROENABLE5VQ8CXP6D:
			boardName = "MicroEnable 5 VQ8-CXP6D";
			break;
		case PN_MICROENABLE5AD8CLHSF2:
			boardName = "MicroEnable 5 AD8-CLHS-F2";
			break;
		default:
			skipIndex = true;
		}

		if (!boardName.empty())
		{
			retVal.push_back(boardName);
		}

		if (!skipIndex) 
		{
			printf("Board ID %2u: %s (%x)\n", i, boardName.c_str(), boardType);
			nrOfBoardsFound++;
		}
		else {

		}
		if (nrOfBoardsFound >= nrOfBoardsPresent) 
		{
			break;// all boards are scanned
		}
	}
#endif
	return retVal;
}
