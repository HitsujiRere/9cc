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

assert 0 "0;"
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
assert 5 "if(1)return 5;return 7;"
assert 7 "if(0)return 5;return 7;"
assert 5 "if(1)return 5;else return 7;"
assert 7 "if(0)return 5;else return 7;"
assert 10 "if(1)return 10;if(1)return 11;return 12;"
assert 10 "if(1)return 10;if(0)return 11;return 12;"
assert 11 "if(0)return 10;if(1)return 11;return 12;"
assert 12 "if(0)return 10;if(0)return 11;return 12;"
assert 10 "if(1)return 10;else if(1)return 11;return 12;"
assert 10 "if(1)return 10;else if(0)return 11;return 12;"
assert 11 "if(0)return 10;else if(1)return 11;return 12;"
assert 12 "if(0)return 10;else if(0)return 11;return 12;"
assert 10 "if(1)return 10;if(1)return 11;else return 12;"
assert 10 "if(1)return 10;if(0)return 11;else return 12;"
assert 11 "if(0)return 10;if(1)return 11;else return 12;"
assert 12 "if(0)return 10;if(0)return 11;else return 12;"
assert 10 "if(1)return 10;else if(1)return 11;else return 12;"
assert 10 "if(1)return 10;else if(0)return 11;else return 12;"
assert 11 "if(0)return 10;else if(1)return 11;else return 12;"
assert 12 "if(0)return 10;else if(0)return 11;else return 12;"
assert 10 "while(1)return 10;return 11;"
assert 11 "while(0)return 10;return 11;"
assert 8 "a=0;while(a < 7)a=a+2;return a;"
assert 11 "while(0)if(1)return 10;return 11;"
assert 11 "while(0)if(0)return 10;return 11;"
assert 10 "while(1)if(1)return 10;else return 11;return 12;"
assert 11 "while(1)if(0)return 10;else return 11;return 12;"
assert 12 "while(0)if(1)return 10;else return 11;return 12;"
assert 12 "while(0)if(0)return 10;else return 11;return 12;"
assert 12 "a=3;while(a>=3)if(a<10)a=a+3;else return a;return 3;"
assert 1 "a=1;for(a=1;a<=3;a=a+1)return a;"
assert 55 "b=0;for(a=1;a<=10;a=a+1)b=a+b;return b;"
assert 30 "b=0;for(a=0;a<20;a=a+1)if(a<10)b=b+1;else b=b+2;return b;"
assert 36 "a=0;for(x=1;x<=3;x=x+1)for(y=1;y<=3;y=y+1)a=a+x+y;return a;"
assert 24 "a=0;for(x=1;x<=3;x=x+1)for(y=x;y<=3;y=y+1)a=a+x+y;return a;"

echo OK
