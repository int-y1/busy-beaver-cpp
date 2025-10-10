Disclaimer: This code is a work in progress, and is not that powerful yet.

C++ version of [`Quick_Sim.py`](https://github.com/sligocki/busy-beaver/blob/main/Code/Quick_Sim.py).

## Windows installation

Go to this repo's directory, then run:

```
wsl
sudo apt update
sudo apt upgrade
sudo apt install make g++ libgmp-dev
make
```

## Example 1

TM that halts in 17825053 steps:
```
./quick_sim 1RB1RA_1RC0RF_0RD---_1LE1LF_1LF1LE_1RA0LD 6
```

`./quick_sim` took ~390ms. `Quick_Sim.py` took ~8.7s.

## Example 2

TM that halts in 987522842126 steps:
```
./quick_sim 1RB2LA1RA_1RC2RB0RC_1LA1RZ1LA 2
```

`./quick_sim` took ~640ms. `Quick_Sim.py` took ~14s.

## Example 3

TM that halts in ~10^1311493 steps:
```
./quick_sim 1RB---_1RC1RB_1RD0RA_1LE0RC_0LF0LD_1LB0LF 4
```

`./quick_sim` cannot do this yet. `Quick_Sim.py` took several hours.
