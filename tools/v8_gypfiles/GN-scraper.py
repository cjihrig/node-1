# Copyright (c) 2019 Refael Ackeramnn<refack@gmail.com>. All rights reserved.
# Use of this source code is governed by an MIT-style license.
import re
import os

PLAIN_SOURCE_RE = re.compile('\s*"([^/$].+)"\s*')
def DoMain(args):
  gn_file, pattern = args
  src_root = os.path.dirname(gn_file)
  with open(gn_file, 'r') as f:
    gn_content = f.read().encode('utf-8')

  scraper_re = re.compile(pattern + r'\[([^\]]+)', re.DOTALL)
  matches = scraper_re.search(gn_content)
  match = matches.group(1)
  files = []
  for l in match.splitlines():
    m2 = PLAIN_SOURCE_RE.match(l)
    if not m2:
      continue
    files.append(m2.group(1))
  # always use `/` since GYP will process paths further downstream
  rel_files = ['"%s/%s"' % (src_root, f) for f in files]
  return ' '.join(rel_files)
