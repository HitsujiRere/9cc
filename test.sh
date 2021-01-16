#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

# assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 7 "6+(10-9);"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 17 "-10+27;"
assert 1 "12==12;"
assert 1 "8 != 9;"
assert 1 "6 <= 9;"
assert 1 "9 <= 9;"
assert 0 "13 <= 7;"
assert 1 "6 < 9;"
assert 0 "9 < 9;"
assert 0 "13 < 7;"
assert 1 "4 + 9 == 7 + 6;"
assert 4 "a=4;"
assert 15 "b=3*5;"
assert 13 "z=6;y=19;x=y-z;"
assert 21 "foo=14;bar=foo/2;foobar=bar+foo;"
assert 10 "return 10;return 20;"
assert 56 "returnx=7;returnreturn=returnx*8;return returnreturn;"

echo OK
