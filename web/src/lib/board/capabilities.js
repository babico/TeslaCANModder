function getNavigatorSerial() {
  if (typeof navigator === 'undefined') {
    return null;
  }

  return navigator.serial || null;
}

function getUserAgent() {
  if (typeof navigator === 'undefined') {
    return '';
  }

  return navigator.userAgent || '';
}

export function getBoardBrowserCapabilities() {
  const userAgent = getUserAgent();
  const canUseWebSerial = Boolean(getNavigatorSerial());
  const isAndroid = /Android/i.test(userAgent);
  const isIPhone = /iPhone/i.test(userAgent);
  const isIPad = /iPad/i.test(userAgent) || (/Macintosh/i.test(userAgent) && typeof navigator !== 'undefined' && navigator.maxTouchPoints > 1);
  const isMobile = /Android|iPhone|iPad|iPod|Mobile/i.test(userAgent);
  const isChromeLike = /Chrome|CriOS|Edg/i.test(userAgent);
  const isAndroidChrome = isAndroid && /Chrome/i.test(userAgent);

  const supportsRuntimeControl = canUseWebSerial && (!isMobile || isAndroidChrome || !isAndroid);
  const supportsFirstFlash = canUseWebSerial && !isMobile;

  // The board protocol is the same on USB and HC-05. This only tells the UI
  // which path to recommend for the current browser and device class.
  let recommendedTransport = 'unsupported';
  if (canUseWebSerial) {
    recommendedTransport = isMobile ? 'bluetooth' : 'usb';
  }

  let compatibilitySummary = 'Use Chrome or Edge on desktop for the full workflow.';
  if (supportsFirstFlash) {
    compatibilitySummary = 'Desktop Chrome or Edge can flash, control, and monitor the board over USB.';
  } else if (supportsRuntimeControl) {
    compatibilitySummary = 'Android Chrome is the target phone path for runtime control through the paired HC-05 serial port.';
  } else if (isIPhone || isIPad) {
    compatibilitySummary = 'This browser can read the guide, but it is not a reliable Web Serial runtime-control path. Use desktop Chrome/Edge or Android Chrome.';
  }

  return {
    canUseWebSerial,
    isMobile,
    isAndroid,
    isIPhone,
    isIPad,
    isAndroidChrome,
    isChromeLike,
    supportsRuntimeControl,
    supportsFirstFlash,
    recommendedTransport,
    compatibilitySummary,
  };
}
