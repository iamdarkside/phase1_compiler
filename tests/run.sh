#!/usr/bin/env bash
set -eu
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
./bminor --ast tests/valid.bminor > "$work/tree"
grep -q 'PrintStmt' "$work/tree"
grep -q 'ReturnStmt' "$work/tree"
./bminor --tokens tests/valid.bminor > "$work/tokens"
grep -q 'string.*hello amirkabir' "$work/tokens"
./bminor --html tests/valid.bminor "$work/out.html"
grep -q 'class="fn">main' "$work/out.html"
grep -q 'hello amirkabir' "$work/out.html"
./bminor --ansi tests/valid.bminor > "$work/ansi"
python3 - "$work/ansi" tests/valid.bminor <<'PY'
import re, sys
with open(sys.argv[1], 'rb') as f: output = f.read()
with open(sys.argv[2], 'rb') as f: source = f.read()
assert re.sub(rb'\x1b\[[0-9;]*m', b'', output) == source
PY
if ./bminor --ast tests/invalid.bminor > "$work/bad-tree" 2> "$work/errors"; then
    echo 'invalid input was accepted' >&2; exit 1
fi
grep -q 'unknown character' "$work/errors"
grep -q 'expected expression' "$work/errors"
grep -q 'second' "$work/bad-tree"
printf 's: string = "unfinished\nnext: integer = 2;\n' > "$work/unclosed"
if ./bminor --tokens "$work/unclosed" > "$work/unclosed-tokens" 2> "$work/unclosed-errors"; then exit 1; fi
grep -q 'unterminated literal' "$work/unclosed-errors"
grep -q 'identifier next' "$work/unclosed-tokens"
printf 'x: integer; /* unfinished' > "$work/comment"
if ./bminor --tokens "$work/comment" > /dev/null 2> "$work/comment-errors"; then exit 1; fi
grep -q 'unterminated block comment' "$work/comment-errors"
echo 'Tests passed.'
python3 - <<'PY'
import subprocess, tempfile
from pathlib import Path

def run(mode, text):
    with tempfile.TemporaryDirectory() as d:
        p = Path(d) / 'input.bminor'
        p.write_text(text)
        return subprocess.run(['./bminor', mode, str(p)], text=True, capture_output=True, timeout=3)

r = run('--tokens', 'return returnValue <= < == = // x\nnext')
assert r.returncode == 0, r.stderr
for part in ['keyword return\n', 'identifier returnValue', 'operator <=', 'operator ==', ':2:1 identifier next']:
    assert part in r.stdout, part
r = run('--ast', 'f: function integer () = { x: integer = 1 + 2 * 3; x = y = 4; return x; }')
assert r.returncode == 0, r.stderr
assert r.stdout.index('BinaryExpr +') < r.stdout.index('BinaryExpr *')
assert r.stdout.count('AssignExpr =') == 2
r = run('--ast', 'a: array [2] integer = {1, 2}; f: function void () = { for (i = 0; i < 2; i++) print g(a[i]); }')
assert r.returncode == 0, r.stderr
for part in ('CallExpr', 'ForStmt', 'ArrayLiteral'):
    assert part in r.stdout, part
r = run('--html', 'main: function void () = { print "<AUT> & hello"; }')
assert r.returncode == 0 and '&lt;AUT&gt; &amp; hello' in r.stdout
r = run('--ast', 'main: function void () = { x: integer = ; print 2; y: integer = ; print 3; }')
assert r.returncode == 1
assert r.stderr.count('expected expression') == 2
assert r.stdout.count('PrintStmt') == 2
r = run('--tokens', "c: char = 'ab'; x: integer; @ y: integer;")
assert r.returncode == 1
assert 'character literal must contain one character' in r.stderr and 'unknown character' in r.stderr
assert 'identifier y' in r.stdout
print('Additional tests passed.')
PY
python3 - <<'PY'
import subprocess, tempfile
from pathlib import Path

with tempfile.TemporaryDirectory() as d:
    p = Path(d) / 'sample.bminor'
    p.write_bytes(b'first: integer;\rsecond: integer;\r\nthird: integer;\nfourth: integer;')
    r = subprocess.run(['./bminor', '--tokens', str(p)], capture_output=True, text=True, timeout=3)
    assert r.returncode == 0, r.stderr
    for expected in (':1:1 identifier first', ':2:1 identifier second', ':3:1 identifier third', ':4:1 identifier fourth'):
        assert expected in r.stdout, expected
    p.write_bytes(b'// comment\rnext: integer;')
    r = subprocess.run(['./bminor', '--tokens', str(p)], capture_output=True, text=True, timeout=3)
    assert r.returncode == 0 and ':2:1 identifier next' in r.stdout
    p.write_text('main: function void () = { x: integer = ; print 2; }')
    r = subprocess.run(['./bminor', '--html', str(p)], capture_output=True, text=True, timeout=3)
    assert r.returncode == 1
    assert '<span class="invalid">;</span>' in r.stdout
    p.write_text('f: function integer () = { f: integer = 1; print f, f(); return 0; }')
    r = subprocess.run(['./bminor', '--html', str(p)], capture_output=True, text=True, timeout=3)
    assert r.returncode == 0, r.stderr
    assert r.stdout.count('class="fn">f</span>') == 2
    assert 'print</span> f, <span class="fn">f</span>' in r.stdout
print('Location and highlighting tests passed.')
PY
