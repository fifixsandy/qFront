OPENQASM 3.0;
include "stdgates.inc";

const uint n=__nondet_uint();

gate cx a,b {} // until external table is not set
gate ccx a,b{} // until external table is not set

gate maj cin, b, a { 
    cx b, a;
    cx b, cin;
    ccx cin, b, a;
}

gate uma cin, b, a { 
    ccx cin, b, a;
    cx a, cin;
    cx cin, b;
}

qubit[n] a;     
qubit[n] b;     
qubit c0;       
qubit z;        