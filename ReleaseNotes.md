###Release v 2.0.6 Byzantine Berserker

* Disabled DNS pulse lookup by default.
* Added a message that warns when try staking with an integrated address.
* Adjusted POS txs drop time in the memory pool.
* Rebranding: 'multisig_monero_tx'-> 'multisig_cutcoin_tx' (issue #3 opened by Satori-Nakamoto 7 Nov 2019).

###Release v 2.0.5 Byzantine Berserker

* New POS statistics output in the cli wallet.
* Fixed bug with 'crypto::secret_key null_skey' deinitialization.
* Continuous rebranding: changed 'monero' -> 'cutcoin' in cle wallet help output.

###Release v 2.0.4 Byzantine Berserker

* Changed the algorithm that estimates the probability of next block forging.
* In the message 'Error: refresh failed: unexpected error: Invalid password: password is needed to compute key image for incoming monero' monero -> cutcoin.
* Corrected the algorithm of time estimation when syncing daemon with the network.
* Always use built-in gtest library for unit tests.

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