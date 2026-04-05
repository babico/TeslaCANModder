import { useState, useCallback, useRef } from 'react';

const initialState = {
  variant: 'hw4',
  hardware: '—',
  driver: '—',
  uptime: 0,
  rate: 0,
  
  fsd: false,
  nag: false,
  profile: 1,
  profilePinned: false,
  offset: 0,
  offsetPinned: false,
  isaChime: false,
  
  canOnline: false,
  standby: false,
  bus2: false,
  
  streaming: false,
  frames: [],
  frameCount: 0,
  
  messages: [],
  
  features: {
    fsd: true,
    profile: true,
    nag: true,
    speedOffset: false,
    isaSpeedChime: false,
    summon: false,
  },
};

export function useBoardState() {
  const [state, setState] = useState(initialState);
  const messageIdRef = useRef(0);

  const addMessage = useCallback((type, text) => {
    const id = ++messageIdRef.current;
    setState(prev => ({
      ...prev,
      messages: [
        {
          id,
          type,
          text,
          ts: new Date().toLocaleTimeString(),
        },
        ...prev.messages,
      ].slice(0, 100),
    }));
  }, []);

  const handleMessage = useCallback((msg) => {
    if (msg.t === 'boot') {
      setState(prev => ({
        ...prev,
        variant: msg.variant || 'hw4',
        hardware: msg.hw || '—',
        driver: msg.drv || '—',
        features: msg.features || prev.features,
        fsd: msg.fsd !== undefined ? Boolean(msg.fsd) : prev.fsd,
        nag: msg.nag !== undefined ? Boolean(msg.nag) : prev.nag,
        profile: msg.sp ?? prev.profile,
        profilePinned: msg.spPin !== undefined ? Boolean(msg.spPin) : prev.profilePinned,
        offset: msg.offset ?? prev.offset,
        offsetPinned: msg.offsetPin !== undefined ? Boolean(msg.offsetPin) : prev.offsetPinned,
        isaChime: msg.isaChime !== undefined ? Boolean(msg.isaChime) : prev.isaChime,
        canOnline: msg.canOnline !== undefined ? Boolean(msg.canOnline) : prev.canOnline,
        standby: msg.standby !== undefined ? Boolean(msg.standby) : prev.standby,
        bus2: msg.bus2 !== undefined ? Boolean(msg.bus2) : prev.bus2,
      }));

      addMessage('info', `Board connected: ${msg.hw}`);
    }
    
    else if (msg.t === 'status') {
      setState(prev => ({
        ...prev,
        variant: msg.variant || prev.variant,
        hardware: msg.hw || prev.hardware,
        driver: msg.drv || prev.driver,
        uptime: msg.up || 0,
        rate: msg.rate || 0,
        fsd: Boolean(msg.fsd),
        nag: Boolean(msg.nag),
        profile: msg.sp ?? 1,
        profilePinned: Boolean(msg.spPin),
        offset: msg.offset ?? 0,
        offsetPinned: Boolean(msg.offsetPin),
        isaChime: Boolean(msg.isaChime),
        streaming: Boolean(msg.stream?.on),
        features: msg.features || prev.features,
        canOnline: msg.canOnline !== undefined ? Boolean(msg.canOnline) : prev.canOnline,
        standby: msg.standby !== undefined ? Boolean(msg.standby) : prev.standby,
        bus2: msg.bus2 !== undefined ? Boolean(msg.bus2) : prev.bus2,
      }));
    }
    
    else if (msg.t === 'frame') {
      setState(prev => {
        const newFrames = [
          {
            key: `${msg.seq}-${msg.id}`,
            id: msg.id,
            dir: msg.dir,
            seq: msg.seq,
            dlc: msg.dlc,
            data: msg.d || '',
            ts: new Date().toLocaleTimeString(),
          },
          ...prev.frames,
        ].slice(0, 100);
        
        return {
          ...prev,
          frames: newFrames,
          frameCount: prev.frameCount + 1,
        };
      });
    }
    
    else if (msg.t === 'ack') {
      addMessage('info', `OK ${msg.cmd}`);
    }
    
    else if (msg.t === 'error') {
      addMessage('error', msg.msg || 'Error');
    }
    
    else if (msg.t === 'log') {
      addMessage('info', msg.msg || 'Log');
    }
    
    else if (msg.t === 'pong') {
      addMessage('info', 'Pong received');
    }
  }, [addMessage]);

  const clearFrames = useCallback(() => {
    setState(prev => ({ ...prev, frames: [], frameCount: 0 }));
  }, []);

  const clearMessages = useCallback(() => {
    setState(prev => ({ ...prev, messages: [] }));
  }, []);

  const reset = useCallback(() => {
    setState(initialState);
  }, []);

  return {
    state,
    handleMessage,
    addMessage,
    clearFrames,
    clearMessages,
    reset,
  };
}
