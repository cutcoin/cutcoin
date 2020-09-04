*** Settings ***
Documentation     Cutcoin tokens integration tests.
Library           Process
Suite Setup       Setup environment
Suite Teardown    Terminate All Processes    kill=True

*** Variables ***
${WORKPATH}       ${CURDIR}/../../cmake-build-debug/bin
${DAEMONPATH}     ${WORKPATH}/cutcoind
${WALLETPATH}     ${WORKPATH}/cutcoin-wallet-cli

${DATAPATH}       ${CURDIR}/data

*** Keywords ***
Setup environment
    Start Process  ${DAEMONPATH}  --data-dir /work/fork_bc/bio1  --p2p-bind-ip 127.0.0.1  --p2p-bind-port 10002  --add-exclusive-node 127.0.0.1:10001  --add-exclusive-node 127.0.0.1:10003  alias=Daemon1
    Start Process  ${DAEMONPATH}  --data-dir /work/fork_bc/bio2  --p2p-bind-ip 127.0.0.1  --p2p-bind-port 10001  --rpc-bind-port 20000  --zmq-rpc-bind-port 30000  --add-exclusive-node 127.0.0.1:10002  alias=Daemon2
    Start Process  ${WALLETPATH}  --wallet-file /work/w/token_staking_2.bin  --daemon-address 127.0.0.1:20000  alias=Wallet
    Sleep  100s

*** Test Cases ***
My Test
    Log    "12345"