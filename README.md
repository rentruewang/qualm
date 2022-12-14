# Quantum Lean Mapper

## This is my personal cleanup of the repository located here: https://github.com/doctry/qft-mapping (of which I'm also a maintainer).

### This repository exists because of some non-fully backwards compatibility changes I made to the code.


# Configurations in the config file

- `algo`: If this configuration is a number $n$, the program maps a QFT of $n$ qubits consisting of only two-qubit gates. If this configuration is a string, then the configuration should be the path to the `.qasm` file.
- `cycle`: The cycle number of different gates.
  - `SINGLE_CYCLE`: The cycle number of single-qubit gates.
  - `SWAP_CYCLE`: The cycle number of swap gates.
  - `CX_CYCLE`: The cycle number of cnot gates.
- `device`: The path to the device `.txt` file.
- `output`: The path to the output `.json` file.
- `mapper`: The configurations about the mapper.
  - `placer`:
    - `placer`:  
  - `placer`:
    - `sa`: Simulated annealing on initial placement.
        - `sa`: Simulated annealing on initial placement. 
    - `sa`: Simulated annealing on initial placement.
    - `dfs`: The order of doing dfs from the qubit 0.
    - `static`: The same as IBM's assignment.
        - `static`: The same as IBM's assignment. 
    - `static`: The same as IBM's assignment.
    - `random`: Randomly placed.
  - `scheduler`:
    - `scheduler`: 
  - `scheduler`:
    - `onion`: Scheduling layer by layer.
    - `static`: Choosing the oldest item in the waitlist.
    - `greedy`: Choosing the best item in the waitlist.
    - `dora`: The exploration scheduler.
    - `old`: The gates are assigned according to the indices of the gates.
    - `random`: Randomly choose an item in the waitlist.
  - `router`:
    - `router`: 
  - `router`:
    - `duostra`: Find the lowest time cost in every step.
    - `apsp`: Find the shortest path(with lowest number of swaps) in every step.
  - `orientation`:
    - `true`: Qubits with small logical indices swapped first.
    - `false`: Default.
  - `cost`:
    - `start`: The cost of a gate is the start time.
    - `end`: The cost of a gate is the end time.
  - `greedy_conf`: The configurations of greedy scheduler.
    - `candidates`: The number of items considered in the waitlist. If the number is less than 1, all the items in the waitlist are considered.
    - `apsp_coef`: The coeffecient dividing apsp cost. Must be integer.
    - `avail`: The available time cost.
      - `min`: The cost of available time is the smaller of the two.
      - `max`: The cost of available time is the greater of the two.
    - `cost`:
        - `cost`: 
    - `cost`:
      - `min`: The min_element will be chosen from the waitlist.
      - `max`: The max_element will be chosen from the waitlist.
    - `layer_from_first`:
      - `true`: The layer of onion scheduler is calculated from the first.
      - `false`: The layer of onion scheduler is calculated from the last.
- `IBM_Gate`:
  - `true`:
  - `false`:
- `stdio`:
  - `true`: Output the result.
  - `false`: Not to output the result.
- `dump`:
  - `true`: Dump the result.
  - `false`: Not to dump the result.
