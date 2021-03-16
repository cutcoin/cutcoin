### Release v 3.0.6
* Fixed bug related to creation of the tokens with hidden supply.

### Release v 3.0.5
* Fix issue in 'sweep_all'
* Implemented tokens with hidden supply. When create a new token it has public supply by default, but now you can add modifier 'hidden' to make it visible only for the owner.

### Release v 3.0.4
* Add mainnet release name

### Release v 3.0.3
* Repair multisignature transactions (for Cutcoins)
* Fix multiple printing / formatting issues on token payments
* Repair input/export blockchain utils
* Add asking about fees when sending tokens
* Add token commands to RPC server

### Release v 3.0.2

* Fix problems with boost 1.7+ build
* Fix building on Windows due to _FORTIFY_SOURCE changes in MSYS2
* Make all token names that start from 'Cut' not allowed in the system
* Optimize token_balance command output

### Release v 3.0.1

* Added RCT format for token genesis tx, this feature improves orerall privacy of the process of tokens creation
* Print token's name in 'show_transfers' command output
* 'token_id' -> 'token_name' in 'create_token' help

### Release v 3.0.0

* Overall, added major functionality that provide support for private tokens. This includes modified bulletproofs for the range commitments, tokens serialization, changes in LMDB, accounts and transactions logic. 
* Added 'create_token' command that creates named token.
* Added 'transfer_token' command that transfers token between wallets.
* Added 'token_balance' command that prints token bakance splits.
* Added 'get_tokens' command that prints known tokens in the system.

### Release v 2.0.6 Byzantine Berserker

* Disabled DNS pulse lookup by default.
* Added a message that warns when try staking with an integrated address.
* Adjusted POS txs drop time in the memory pool.
* Rebranding: 'multisig_monero_tx'-> 'multisig_cutcoin_tx' (issue #3 opened by Satori-Nakamoto 7 Nov 2019).

### Release v 2.0.5 Byzantine Berserker

* New POS statistics output in the cli wallet.
* Fixed bug with 'crypto::secret_key null_skey' deinitialization.
* Continuous rebranding: changed 'monero' -> 'cutcoin' in cle wallet help output.

### Release v 2.0.4 Byzantine Berserker

* Changed the algorithm that estimates the probability of next block forging.
* In the message 'Error: refresh failed: unexpected error: Invalid password: password is needed to compute key image for incoming monero' monero -> cutcoin.
* Corrected the algorithm of time estimation when syncing daemon with the network.
* Always use built-in gtest library for unit tests.

### Release v 2.0.3 Byzantine Berserker

* Rewritten RPC command 'status' in the daemon'.
* 'internal error: try to insert duplicate iterator in key_image set' error downgraded to a warning.

### Release v 2.0.2 Byzantine Berserker

* Added support for GUI wallet.
* Output detailed POS status and statistics when staking. Available both in cli and GUI wallet.
* Removed 'start_mining' and 'stop_mining' commands in the daemon.
* Added support for transferring of the POS reward in a different (from the staking one) wallet.
* Disabled fluffy blocks by default
* Changed firewall detection interval 15min -> 2h.
* Removed hardcoded Monero constants in 'is_tx_spendtime_unlocked'
* Couple of minor fixes / changes.

### Release v 2.0.0 Byzantine Berserker 25 Feb 2019

Switch to Proof of Stake consensus