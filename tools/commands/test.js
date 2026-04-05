/** test command — FSD / profile functional round-trip tests. */

import { setTimeout as delay } from 'node:timers/promises';
import { parseHexData, compareBits } from './watch.js';

export async function runTest(session, opts, out) {
  const { timeoutMs, testFsd, testProfile, testFsdListen, restore } = opts;

  session.send('status');
  const baseline = await session.waitForType('status', timeoutMs);
  const origFsd = baseline ? Number(baseline.msg.fsd) : null;
  const origProfile = baseline ? Number(baseline.msg.sp) : null;

  if (baseline) out.info(`Baseline — fsd=${origFsd} sp=${origProfile} variant=${baseline.msg.variant}`);
  else out.warn('No baseline status received — results may be unreliable');

  if (testFsd) {
    out.section('FSD round-trip test');

    session.send('fsd:on');
    const onAck = await session.waitForAck('fsd:on', timeoutMs);
    onAck ? out.pass('fsd:on acknowledged') : out.fail('fsd:on', 'no ack');

    await delay(600);
    session.send('status');
    const afterOn = await session.waitForType('status', timeoutMs);
    if (afterOn) {
      Number(afterOn.msg.fsd) === 1
        ? out.pass('FSD enabled in status', `fsd=${afterOn.msg.fsd}`)
        : out.fail('FSD on verification', `fsd=${afterOn.msg.fsd} (expected 1)`);
    } else out.fail('FSD on verification', 'no status after fsd:on');

    session.send('fsd:off');
    const offAck = await session.waitForAck('fsd:off', timeoutMs);
    offAck ? out.pass('fsd:off acknowledged') : out.fail('fsd:off', 'no ack');

    await delay(600);
    session.send('status');
    const afterOff = await session.waitForType('status', timeoutMs);
    if (afterOff) {
      Number(afterOff.msg.fsd) === 0
        ? out.pass('FSD disabled in status', `fsd=${afterOff.msg.fsd}`)
        : out.fail('FSD off verification', `fsd=${afterOff.msg.fsd} (expected 0)`);
    } else out.fail('FSD off verification', 'no status after fsd:off');

    if (restore && origFsd !== null) {
      session.send(origFsd === 1 ? 'fsd:on' : 'fsd:off');
      await delay(400);
      out.info(`Restored fsd to ${origFsd}`);
    }
  }

  if (testProfile >= 0) {
    out.section(`Profile round-trip test (target=${testProfile})`);

    const cmd = `profile:${testProfile}`;
    session.send(cmd);
    const ack = await session.waitForAck(cmd, timeoutMs);
    ack ? out.pass(`${cmd} acknowledged`) : out.fail(cmd, 'no ack');

    await delay(700);
    session.send('status');
    const afterSet = await session.waitForType('status', timeoutMs);
    if (afterSet) {
      Number(afterSet.msg.sp) === testProfile
        ? out.pass(`Profile ${testProfile} confirmed`, `sp=${afterSet.msg.sp}`)
        : out.fail(`Profile ${testProfile} verification`, `sp=${afterSet.msg.sp} (expected ${testProfile})`);
    } else out.fail('Profile verification', 'no status after profile command');

    if (restore && origProfile !== null) {
      session.send(`profile:${origProfile}`);
      await delay(400);
      out.info(`Restored profile to ${origProfile}`);
    }
  }

  if (testFsdListen) {
    const dur = Number(opts.watchDurMs) || 3000;
    out.section(`FSD activate + listen (${dur}ms, no restore)`);

    session.send('fsd:on');
    const onAck = await session.waitForAck('fsd:on', timeoutMs);
    onAck ? out.pass('fsd:on acknowledged') : out.fail('fsd:on', 'no ack');

    await delay(600);
    session.send('status');
    const afterOn = await session.waitForType('status', timeoutMs);
    if (afterOn) {
      Number(afterOn.msg.fsd) === 1
        ? out.pass('FSD enabled', `fsd=${afterOn.msg.fsd}`)
        : out.fail('FSD activation', `fsd=${afterOn.msg.fsd}`);
    }

    session.send('stream:on');
    await delay(200);

    let frameCount = 0;
    const listenEnd = Date.now() + dur;
    while (Date.now() < listenEnd) {
      const entry = await session.waitFor(() => true, Math.max(100, listenEnd - Date.now()));
      if (!entry) break;
      if (entry.msg?.t === 'frame') frameCount++;
    }

    session.send('stream:off');
    await delay(200);
    out.pass(`Listened for ${dur}ms`, `${frameCount} frames observed`);
    out.info('FSD state NOT restored (as requested)');
  }

  if (!testFsd && testProfile < 0 && !testFsdListen) {
    out.info('No test specified. Use --test-fsd, --test-profile <n>, or --test-fsd-listen');
  }
}
