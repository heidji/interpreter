## Installation / How to build
```bash
sudo apt-get install php7.1-dev
git clone https://github.com/CopernicaMarketingSoftware/PHP-CPP.git
make
sudo make install
git clone https://github.com/heidji/interpreter.git
cd interpreter
make -B
```

The generated `interpreter.so` file can included as follows:
Add an `interpreter`.ini file in the `/etc/php/7.3/mods-available` folder with the following content
`extension=/full/path/to/so/example.so`

## Optimization ideas:
- create another event array with their typeIds as the key to each subset. this should optimize the primary condition loop because it would be clear which typeId they're looking for, and would reduce the times significantly.
- create evals on `eq[name].event.evals` level for their qualifiers. this prevents the logic eval from attempting to re-match the qualifier which has been already rejected at a prior stage, if it can check a flag for that.
- unfold the formula.
- check the fact that qualifier are now in the evals (also the single ones for the primary). remove it and implement step 2. in this list before and use this instead. for the sorted_args eval, you can check if the arg is in the condition list && is not event => primary event qualifier.