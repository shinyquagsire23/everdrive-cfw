#include <stdbool.h>

#include "sys.h"
#include "bios.h"
#include "disk.h"

static u32 bHasInit = 0;

/*-----------------------------------------------------------------
startUp
Initialize the interface, geting it into an idle, ready state
returns true if successful, otherwise returns false
-----------------------------------------------------------------*/
bool EDIO_startup(void) {
    //if (bHasInit) return true;

    bi_init();

    u8 resp = diskInit();
    if (resp)return false;
    
    bHasInit = 1;
    
	return true;
}

/*-----------------------------------------------------------------
isInserted
Is a card inserted?
return true if a card is inserted and usable
-----------------------------------------------------------------*/
bool EDIO_isInserted (void) {
    //TODO find a good heuristic for this
	return true;
}


/*-----------------------------------------------------------------
clearStatus
Reset the card, clearing any status errors
return  true if the card is idle and ready
-----------------------------------------------------------------*/
bool EDIO_clearStatus (void) {
	return true;
}


/*-----------------------------------------------------------------
readSectors
Read "numSectors" 512-byte sized sectors from the card into "buffer", 
starting at "sector".
return true if it was successful, false if it failed for any reason
-----------------------------------------------------------------*/
bool EDIO_readSectors (u32 sector, u32 numSectors, void* buffer) {
	u8* data = (u8*)buffer;
	
	u8 resp = diskRead(sector, data, numSectors);
    if (resp)return false;
	
	return true;
}



/*-----------------------------------------------------------------
writeSectors
Write "numSectors" 512-byte sized sectors from "buffer" to the card, 
starting at "sector".
return true if it was successful, false if it failed for any reason
-----------------------------------------------------------------*/
bool EDIO_writeSectors (u32 sector, u32 numSectors, void* buffer) {
	u8* data = (u8*)buffer;
	
	u8 resp = diskWrite(sector, data, numSectors);
    if (resp)return false;
	
	return true;
}

/*-----------------------------------------------------------------
shutdown
shutdown the card, performing any needed cleanup operations
-----------------------------------------------------------------*/
bool EDIO_shutdown(void) {
    //if (!bHasInit) return true;

    bHasInit = 0;

    diskPowerDown();
    bi_shutdown();
	return true;
}
