const fs = require('fs');
const path = require('path');

const legacyDir = 'C:/Users/boran/Masaüstü/Tesla-CAN-Mod/docs/legacy';
const files = fs.readdirSync(legacyDir).filter(f => f.endsWith('.md'));
let count = 0;

for (const f of files) {
  const fp = path.join(legacyDir, f);
  let c = fs.readFileSync(fp, 'utf8');
  if (c.startsWith('---')) continue;

  const h1 = (c.match(/^# (.+)/m) || [])[1] || f.replace('.md', '');
  const overviewMatch = c.match(/## Overview\s*\n+([^\n#]+)/);
  const desc = overviewMatch
    ? overviewMatch[1].trim().slice(0, 120).replace(/:/g, ' -')
    : 'Community Tesla CAN project reference.';

  const slug = f.replace('.md', '');
  const dashIdx = slug.indexOf('-');
  const author = dashIdx > -1 ? slug.slice(0, dashIdx) : slug;
  const repo = dashIdx > -1 ? slug.slice(dashIdx + 1) : slug;

  const fm = [
    '---',
    'title: ' + h1.trim(),
    'description: ' + desc,
    'category: legacy',
    'folder: legacy',
    'tags: [legacy, community, external]',
    'author: ' + author,
    'repo: ' + repo,
    '---',
    '',
    '',
  ].join('\n');

  fs.writeFileSync(fp, fm + c);
  count++;
}

console.log('Done -', count, 'files updated');
