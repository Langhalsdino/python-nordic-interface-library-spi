import nordicUtils as nu
import time

class Bumblebee:
    # callbacks
    batteryLowCallbacks = []
    
    # functions
    def spiParseStatusReg(self, status):
        # check important error bits and call callbacks
        if status & (1 << 1):
            for cb in batteryLowCallbacks:
                try:
                    cb()
                except:
                    pass
        return 0
    
    def spiTransfer(self, commands):
        # transfer
        ret = nu.SpiXfer(commands)
        time.sleep(0.001)
        
        # parse status
        self.spiParseStatusReg(ret[0])
        
        return ret
        
    
    def spiWaitForIdle(self, timeoutMs):
        startTime = time.time()
        
        while ( True ):
            status = self.spiTransfer([0x00])[0]
            
            if (status & (1 << 7)):
                return 0
            
            if ((time.time() - startTime) > (timeoutMs/1000)):
                return 1
                
    def spiWaitForDataReady(self, timeoutMs):
        startTime = time.time()
        
        while ( True ):
            status = self.spiTransfer([0x00])[0]
            
            if (status & (1 << 6)):
                return 0
            
            if ((time.time() - startTime) > (timeoutMs/1000)):
                return 1
    
    def spiExecCommand(self, command):
        if ( self.spiWaitForIdle(1000) ):
            # not ready
            return 1
        
        # write command
        self.spiTransfer([command])
        
        # wait for cts flag to be reset
        if ( self.spiWaitForIdle(100) ):
            return 1
        
        return 0
    
    def spiWriteVal(self, id, val):
        return 0
    
    def spiReadVal16(self, id):
        self.spiExecCommand(id)
        self.spiWaitForDataReady(1000)
        readOut = self.spiTransfer([0x02, 0, 0])
        return (readOut[2]*256+readOut[1])