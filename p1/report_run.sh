#!/bin/bash

alphal=(0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9)
tslicel=(10 20 30)
rraddl=("BEGINNING", "END")

for alpha in ${alphal[*]}; do
  for tslice in ${tslicel[*]}; do
    for rradd in ${rraddl[*]}; do
        python project_report.py 2 0.01 200 20 20 $alpha $tslice $rradd
    done
  done
done

