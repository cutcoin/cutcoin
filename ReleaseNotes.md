###Release v 2.0.3 Byzantine Berserker

* Rewritten RPC command 'status' in the daemon'.
* 'internal error: try to insert duplicate iterator in key_image set' error downgraded to a warning.

###Release v 2.0.2 Byzantine Berserker

* Added support for GUI wallet.
* Output detailed POS status and statistics when staking. Available both in cli and GUI wallet.
* Removed 'start_mining' and 'stop_mining' commands in the daemon.
* Added support for transferring of the POS reward in a different (from the staking one) wallet.
* Disabled fluffy blocks by default
* Changed firewall detection interval 15min -> 2h.
* Removed hardcoded Monero constants in 'is_tx_spendtime_unlocked'
* Couple of minor fixes / changes.

###Release v 2.0.0 Byzantine Berserker 25 Feb 2019

Switch to Proof of Stake consensus