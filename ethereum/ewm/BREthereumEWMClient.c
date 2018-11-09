//
//  BREthereumEWMClient.c
//  BRCore
//
//  Created by Ed Gamble on 5/7/18.
//  Copyright (c) 2018 breadwallet LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "BRArray.h"
#include "BREthereumEWMPrivate.h"

// We use private BCS interfaces to 'inject' our JSON_RPC 'announced' results as if from LES
#include "../bcs/BREthereumBCSPrivate.h"

//
//
//
//

// ==============================================================================================
//
// Wallet Balance
//

extern void
ewmUpdateWalletBalance(BREthereumEWM ewm,
                       BREthereumWalletId wid) {
    BREthereumWallet wallet = ewmLookupWallet(ewm, wid);
    
    if (NULL == wallet) {
        ewmClientSignalWalletEvent(ewm, wid, WALLET_EVENT_BALANCE_UPDATED,
                                     ERROR_UNKNOWN_WALLET,
                                     NULL);
        
    } else if (ETHEREUM_BOOLEAN_IS_FALSE(ewmIsConnected(ewm))) {
        ewmClientSignalWalletEvent(ewm, wid, WALLET_EVENT_BALANCE_UPDATED,
                                     ERROR_NODE_NOT_CONNECTED,
                                     NULL);
    } else {
        switch (ewm->mode) {
            case BRD_ONLY:
            case BRD_WITH_P2P_SEND: {
                char *address = addressGetEncodedString(walletGetAddress(wallet), 0);

                ewm->client.funcGetBalance (ewm->client.context,
                                            ewm,
                                            wid,
                                            address,
                                            ++ewm->requestId);

                free(address);
                break;
            }

            case P2P_WITH_BRD_SYNC:
            case P2P_ONLY:
                // TODO: LES Update Wallet Balance
                break;
        }
    }
}

/**
 * Handle the Client Announcement for the balance of `wallet`.  This will be the BRD Backend's
 * result (not necessarily a JSON_RPC result) for the balance which will be ether ETH or TOK.
 *
 * @param ewm
 * @param wallet
 * @param value
 * @param rid
 */
extern void
ewmClientHandleAnnounceBalance (BREthereumEWM ewm,
                                BREthereumWallet wallet,
                                UInt256 value,
                                int rid) {
    BREthereumAmount amount =
    (AMOUNT_ETHER == walletGetAmountType(wallet)
     ? amountCreateEther(etherCreate(value))
     : amountCreateToken(createTokenQuantity(walletGetToken(wallet), value)));

    ewmSignalBalance(ewm, amount);
}

// ==============================================================================================
//
// Default Wallet Gas Price
//
extern void
ewmUpdateWalletDefaultGasPrice (BREthereumEWM ewm,
                                BREthereumWalletId wid) {
    BREthereumWallet wallet = ewmLookupWallet(ewm, wid);
    
    if (NULL == wallet) {
        ewmClientSignalWalletEvent(ewm, wid, WALLET_EVENT_DEFAULT_GAS_PRICE_UPDATED,
                                     ERROR_UNKNOWN_WALLET,
                                     NULL);
        
    } else if (ETHEREUM_BOOLEAN_IS_FALSE(ewmIsConnected(ewm))) {
        ewmClientSignalWalletEvent(ewm, wid, WALLET_EVENT_DEFAULT_GAS_PRICE_UPDATED,
                                     ERROR_NODE_NOT_CONNECTED,
                                     NULL);
    } else {
        switch (ewm->mode) {
            case BRD_ONLY:
            case BRD_WITH_P2P_SEND: {
                ewm->client.funcGetGasPrice (ewm->client.context,
                                             ewm,
                                             wid,
                                             ++ewm->requestId);
                break;
            }

            case P2P_WITH_BRD_SYNC:
            case P2P_ONLY:
                // TODO: LES Update Wallet Balance
                break;
        }
    }
}

extern void
ewmClientHandleAnnounceGasPrice (BREthereumEWM ewm,
                                 BREthereumWallet wallet,
                                 UInt256 amount,
                                 int rid) {
    ewmSignalGasPrice(ewm, wallet, gasPriceCreate(etherCreate(amount)));
}

// ==============================================================================================
//
// Transaction Gas Estimate
//

extern void
ewmUpdateTransferGasEstimate (BREthereumEWM ewm,
                                 BREthereumWalletId wid,
                                 BREthereumTransferId tid) {
    BREthereumTransfer transfer = ewmLookupTransfer(ewm, tid);
    
    if (NULL == transfer) {
        ewmClientSignalTransferEvent(ewm, wid, tid,
                                          TRANSFER_EVENT_GAS_ESTIMATE_UPDATED,
                                          ERROR_UNKNOWN_WALLET,
                                          NULL);
        
    } else if (ETHEREUM_BOOLEAN_IS_FALSE(ewmIsConnected(ewm))) {
        ewmClientSignalTransferEvent(ewm, wid, tid,
                                          TRANSFER_EVENT_GAS_ESTIMATE_UPDATED,
                                          ERROR_NODE_NOT_CONNECTED,
                                          NULL);
    } else {
        switch (ewm->mode) {
            case BRD_ONLY:
            case BRD_WITH_P2P_SEND: {
                // This will be ZERO if transaction amount is in TOKEN.
                BREthereumEther amountInEther = transferGetEffectiveAmountInEther(transfer);
                BREthereumTransaction transaction = transferGetOriginatingTransaction(transfer);
                char *to = (char *) addressGetEncodedString(transactionGetTargetAddress(transaction), 0);
                char *amount = coerceString(amountInEther.valueInWEI, 16);

                ewm->client.funcEstimateGas (ewm->client.context,
                                             ewm,
                                             wid,
                                             tid,
                                             to,
                                             amount,
                                             transactionGetData(transaction),
                                             ++ewm->requestId);

                free(to);
                free(amount);
                break;
            }

            case P2P_WITH_BRD_SYNC:
            case P2P_ONLY:
                // TODO: LES Update Wallet Balance
                break;
        }
    }

}

extern void
ewmClientHandleAnnounceGasEstimate (BREthereumEWM ewm,
                                    BREthereumWallet wallet,
                                    BREthereumTransfer transfer,
                                    UInt256 value,
                                    int rid) {
    ewmSignalGasEstimate(ewm, wallet, transfer, gasCreate(value.u64[0]));
}

// ==============================================================================================
//
// Get Block Number
//
extern void
ewmUpdateBlockNumber (BREthereumEWM ewm) {
    if (ETHEREUM_BOOLEAN_IS_FALSE(ewmIsConnected(ewm))) return;
    switch (ewm->mode) {
        case BRD_ONLY:
        case BRD_WITH_P2P_SEND: {
            ewm->client.funcGetBlockNumber (ewm->client.context,
                                            ewm,
                                            ++ewm->requestId);
            break;
        }

        case P2P_WITH_BRD_SYNC:
        case P2P_ONLY:
            // TODO: LES Update Wallet Balance
            break;
    }
}

/**
 * Handle a Client Announcement of `blockNumber`.  This will be BRD Backend's JSON_RPC result for
 * the most recent Ethereum blockNumber
 *
 * @param ewm
 * @param blockNumber
 * @param rid
 */
extern void
ewmClientHandleAnnounceBlockNumber (BREthereumEWM ewm,
                                    uint64_t blockNumber,
                                    int rid) {
    ewmUpdateBlockHeight(ewm, blockNumber);
}

// ==============================================================================================
//
// Get Nonce
//
extern void
ewmUpdateNonce (BREthereumEWM ewm) {
    if (ETHEREUM_BOOLEAN_IS_FALSE(ewmIsConnected(ewm))) return;
    switch (ewm->mode) {
        case BRD_ONLY:
        case BRD_WITH_P2P_SEND: {
            char *address = addressGetEncodedString(accountGetPrimaryAddress(ewm->account), 0);

            ewm->client.funcGetNonce (ewm->client.context,
                                      ewm,
                                      address,
                                      ++ewm->requestId);

            free (address);
            break;
        }

        case P2P_WITH_BRD_SYNC:
        case P2P_ONLY:
            // TODO: LES Update Wallet Balance
            break;
    }
}

/**
 * Handle a Client Announcement of `nonce`.  This will be the BRD Backend's JSON_RPC result for
 * the the address' nonce.
 *
 * @param ewm
 * @param address
 * @param nonce
 * @param rid
 */
extern void
ewmClientHandleAnnounceNonce (BREthereumEWM ewm,
                              BREthereumAddress address,
                              uint64_t nonce,
                              int rid) {
    //    BREthereumEncodedAddress address = accountGetPrimaryAddress (ewmGetAccount(ewm));
    //    assert (ETHEREUM_BOOLEAN_IS_TRUE (addressHasString(address, strAddress)));
    accountSetAddressNonce(ewm->account, accountGetPrimaryAddress(ewm->account), nonce, ETHEREUM_BOOLEAN_FALSE);
}

// ==============================================================================================
//
// Get Transactions
//
extern void
ewmUpdateTransactions (BREthereumEWM ewm) {
    if (ETHEREUM_BOOLEAN_IS_FALSE(ewmIsConnected(ewm))) {
        // Nothing to announce
        return;
    }
    switch (ewm->mode) {
        case BRD_ONLY:
        case BRD_WITH_P2P_SEND: {
            char *address = addressGetEncodedString(accountGetPrimaryAddress(ewm->account), 0);

            ewm->client.funcGetTransactions (ewm->client.context,
                                             ewm,
                                             address,
                                             ++ewm->requestId);

            free (address);
            break;
        }

        case P2P_WITH_BRD_SYNC:
        case P2P_ONLY:
            // TODO: LES Update Wallet Balance
            break;
    }
}

extern void
ewmClientHandleAnnounceTransaction(BREthereumEWM ewm,
                                   BREthereumEWMClientAnnounceTransactionBundle *bundle,
                                   int id) {
    switch (ewm->mode) {
        case BRD_ONLY:
        case BRD_WITH_P2P_SEND: {
            //
            // This 'annouce' call is coming from the guaranteed BRD endpoint; thus we don't need to
            // worry about the validity of the transaction - is is surely confirmed.  Is that true
            // if newly submitted?

            // TODO: Confirm we are not repeatedly creating transactions
            BREthereumTransaction transaction = transactionCreate (bundle->from,
                                                                   bundle->to,
                                                                   etherCreate(bundle->amount),
                                                                   gasPriceCreate(etherCreate(bundle->gasPrice)),
                                                                   gasCreate(bundle->gasLimit),
                                                                   bundle->data,
                                                                   bundle->nonce);

            // We set the transaction's hash based on the value providedin the bundle.  However,
            // and importantly, if we attempted to compute the hash - as we normally do for a
            // signed transaction - the computed hash would be utterly wrong.  The transaction
            // just created does not have: network nor signature.
            //
            // TODO: Confirm that BRPersistData does not overwrite the transaction's hash
            transactionSetHash (transaction, bundle->hash);

            BREthereumTransactionStatus status = transactionStatusCreateIncluded (gasCreate(bundle->gasUsed),
                                                                                  bundle->blockHash,
                                                                                  bundle->blockNumber,
                                                                                  bundle->blockTransactionIndex);
            transactionSetStatus (transaction, status);

            // If we had a `bcs` we might think about `bcsSignalTransaction(ewm->bcs, transaction);`
            ewmSignalTransaction(ewm, BCS_CALLBACK_TRANSACTION_UPDATED, transaction);

            break;
        }

        case P2P_WITH_BRD_SYNC:
        case P2P_ONLY:
            bcsSendTransactionRequest(ewm->bcs,
                                      bundle->hash,
                                      bundle->blockNumber,
                                      bundle->blockTransactionIndex);
            break;
    }
    ewmClientAnnounceTransactionBundleRelease(bundle);
}



// ==============================================================================================
//
// Get Logs
//
static const char *
ewmGetWalletContractAddress (BREthereumEWM ewm, BREthereumWalletId wid) {
    BREthereumWallet wallet = ewmLookupWallet(ewm, wid);
    if (NULL == wallet) return NULL;
    
    BREthereumToken token = walletGetToken(wallet);
    return (NULL == token ? NULL : tokenGetAddress(token));
}

extern void
ewmUpdateLogs (BREthereumEWM ewm,
               BREthereumWalletId wid,
               BREthereumContractEvent event) {
    if (ETHEREUM_BOOLEAN_IS_FALSE(ewmIsConnected(ewm))) {
        // Nothing to announce
        return;
    }
    switch (ewm->mode) {
        case BRD_ONLY:
        case BRD_WITH_P2P_SEND: {
            char *address = addressGetEncodedString(accountGetPrimaryAddress(ewm->account), 0);
            char *encodedAddress =
            eventERC20TransferEncodeAddress (event, address);
            const char *contract =ewmGetWalletContractAddress(ewm, wid);

            ewm->client.funcGetLogs (ewm->client.context,
                                     ewm,
                                     contract,
                                     encodedAddress,
                                     eventGetSelector(event),
                                     ++ewm->requestId);

            free (encodedAddress);
            free (address);
            break;
        }

        case P2P_WITH_BRD_SYNC:
        case P2P_ONLY:
            // TODO: LES Update Logs
            break;
    }
}

extern void
ewmClientHandleAnnounceLog (BREthereumEWM ewm,
                            BREthereumEWMClientAnnounceLogBundle *bundle,
                            int id) {
    switch (ewm->mode) {
        case BRD_ONLY:
        case BRD_WITH_P2P_SEND: {
            // This 'announce' call is coming from the guaranteed BRD endpoint; thus we don't need to
            // worry about the validity of the transaction - is is surely confirmed.  Is that true
            // if newly submitted?

            BREthereumLogTopic topics [bundle->topicCount];
            for (size_t index = 0; index < bundle->topicCount; index++)
                topics[index] = logTopicCreateFromString(bundle->arrayTopics[index]);

            BREthereumLog log = logCreate(bundle->contract,
                                          bundle->topicCount,
                                          topics);
            logInitializeIdentifier(log, bundle->hash, bundle->logIndex);

            BREthereumTransactionStatus status =
            transactionStatusCreateIncluded(gasCreate(0),
                                            hashCreateEmpty(),
                                            bundle->blockNumber,
                                            bundle->blockTransactionIndex);
            logSetStatus(log, status);

            // If we had a `bcs` we might think about `bcsSignalLog(ewm->bcs, log);`
            ewmSignalLog(ewm, BCS_CALLBACK_LOG_UPDATED, log);

            // Do we need a transaction here?  No, if another originated this Log, then we can't ever
            // care and if we originated this Log, then we'll get the transaction (as part of the BRD
            // 'getTransactions' endpoint).
            //
            // Of course, see the comment in bcsHandleLog asking how to tell the client about a Log...
            break;
        }

        case P2P_WITH_BRD_SYNC:
        case P2P_ONLY:
            bcsSendLogRequest(ewm->bcs,
                              bundle->hash,
                              bundle->blockNumber,
                              bundle->blockTransactionIndex);
            break;
    }
    ewmClientAnnounceLogBundleRelease(bundle);
}

// ==============================================================================================
//
// Submit Transaction
//

extern void // status, error
ewmWalletSubmitTransfer(BREthereumEWM ewm,
                        BREthereumWalletId wid,
                        BREthereumTransferId tid) {
    BREthereumWallet wallet = ewmLookupWallet(ewm, wid);
    BREthereumTransfer transfer = ewmLookupTransfer(ewm, tid);
    // assert wallet-has-transfer
    // assert signed

    BREthereumTransaction transaction = transferGetOriginatingTransaction(transfer);
    BREthereumBoolean isSigned = transactionIsSigned (transaction);


    switch (ewm->mode) {
        case BRD_ONLY: {
            char *rawTransaction = transactionGetRlpHexEncoded (transaction,
                                                                ewm->network,
                                                                (ETHEREUM_BOOLEAN_IS_TRUE (isSigned)
                                                                 ? RLP_TYPE_TRANSACTION_SIGNED
                                                                 : RLP_TYPE_TRANSACTION_UNSIGNED),
                                                                "0x");

            ewm->client.funcSubmitTransaction (ewm->client.context,
                                               ewm,
                                               ewmLookupWalletId(ewm, wallet),
                                               ewmLookupTransferId(ewm, transfer),
                                               rawTransaction,
                                               ++ewm->requestId);

            free(rawTransaction);
            break;
        }

        case BRD_WITH_P2P_SEND:
        case P2P_WITH_BRD_SYNC:
        case P2P_ONLY:
            bcsSendTransaction(ewm->bcs, transaction);
            break;
    }
}


extern void
ewmClientHandleAnnounceSubmitTransfer (BREthereumEWM ewm,
                                       BREthereumWallet wallet,
                                       BREthereumTransfer transfer,
                                       int rid) {
    BREthereumTransaction transaction = transferGetOriginatingTransaction (transfer);
    assert (NULL != transaction);

    switch (ewm->mode) {
        case BRD_ONLY:
            assert (0);

        case BRD_WITH_P2P_SEND:
        case P2P_WITH_BRD_SYNC:
        case P2P_ONLY:
            transactionSetStatus (transaction, transactionStatusCreate(TRANSACTION_STATUS_PENDING));

            // If we had a `bcs` we might think about `bcsSignalTransaction(ewm->bcs, transaction);`
            ewmSignalTransaction (ewm, BCS_CALLBACK_TRANSACTION_ADDED, transaction);
            break;
    }
}

// ==============================================================================================
//
// Update Tokens
//
extern void
ewmUpdateTokens (BREthereumEWM ewm) {
    ewm->client.funcGetTokens
    (ewm->client.context,
     ewm,
     ++ewm->requestId);
}

extern void
ewmClientHandleAnnounceToken (BREthereumEWM ewm,
                              BREthereumEWMClientAnnounceTokenBundle *bundle,
                              int id) {
    tokenInstall (bundle->address,
                  bundle->symbol,
                  bundle->name,
                  bundle->description,
                  bundle->decimals,
                  bundle->gasLimit,
                  bundle->gasPrice);

    ewmClientAnnounceTokenBundleRelease(bundle);
}


// ==============================================================================================
//
// Client
//

extern void
ewmClientHandleWalletEvent(BREthereumEWM ewm,
                           BREthereumWalletId wid,
                           BREthereumWalletEvent event,
                           BREthereumStatus status,
                           const char *errorDescription) {
    ewm->client.funcWalletEvent
    (ewm->client.context,
     ewm,
     wid,
     event,
     status,
     errorDescription);
}

extern void
ewmClientHandleBlockEvent(BREthereumEWM ewm,
                          BREthereumBlockId bid,
                          BREthereumBlockEvent event,
                          BREthereumStatus status,
                          const char *errorDescription) {
    ewm->client.funcBlockEvent (ewm->client.context,
                                ewm,
                                bid,
                                event,
                                status,
                                errorDescription);
}

extern void
ewmClientHandleTransferEvent (BREthereumEWM ewm,
                              BREthereumWalletId wid,
                              BREthereumTransferId tid,
                              BREthereumTransferEvent event,
                              BREthereumStatus status,
                              const char *errorDescription) {

    if (TRANSFER_EVENT_GAS_ESTIMATE_UPDATED != event &&
        TRANSFER_EVENT_BLOCK_CONFIRMATIONS_UPDATED != event) {
        BREthereumTransfer transfer = ewm->transfers[tid];
        BREthereumTransaction transaction = transferGetBasisTransaction (transfer);
        BREthereumLog log = transferGetBasisLog(transfer);
        assert (NULL == transaction || NULL == log);

        BREthereumClientChangeType type = (event == TRANSFER_EVENT_CREATED
                                           ? CLIENT_CHANGE_ADD
                                           : (event == TRANSFER_EVENT_DELETED
                                              ? CLIENT_CHANGE_REM
                                              : CLIENT_CHANGE_UPD));

        BRRlpItem item = (NULL != transaction
                          ? transactionRlpEncode (transaction, ewm->network, RLP_TYPE_ARCHIVE, ewm->coder)
                          : logRlpEncode(log, RLP_TYPE_ARCHIVE, ewm->coder));

        BREthereumHash hash = (NULL != transaction
                               ? transactionGetHash(transaction)
                               : logGetHash(log));

        // Notice the final '1' - don't release `data`...
        BREthereumHashDataPair pair = hashDataPairCreate (hash,
                                                          dataCreateFromRlpData(rlpGetData(ewm->coder, item), 1));

        rlpReleaseItem(ewm->coder, item);

        if (NULL != transaction)
            ewm->client.funcChangeTransaction (ewm->client.context, ewm, type, pair);
        else
            ewm->client.funcChangeLog (ewm->client.context, ewm, type, pair);
    }

    ewm->client.funcTransferEvent (ewm->client.context,
                                   ewm,
                                   wid,
                                   tid,
                                   event,
                                   status,
                                   errorDescription);
}

extern void
ewmClientHandlePeerEvent(BREthereumEWM ewm,
                         // BREthereumWalletId wid,
                         // BREthereumTransactionId tid,
                         BREthereumPeerEvent event,
                         BREthereumStatus status,
                         const char *errorDescription) {
    ewm->client.funcPeerEvent (ewm->client.context,
                               ewm,
                               // event->wid,
                               // event->tid,
                               event,
                               status,
                               errorDescription);
}

extern void
ewmClientHandleEWMEvent(BREthereumEWM ewm,
                        // BREthereumWalletId wid,
                        // BREthereumTransactionId tid,
                        BREthereumEWMEvent event,
                        BREthereumStatus status,
                        const char *errorDescription) {
    ewm->client.funcEWMEvent (ewm->client.context,
                              ewm,
                              //event->wid,
                              // event->tid,
                              event,
                              status,
                              errorDescription);
}



#if 0 // Transaction
BREthereumTransactionId tid = -1;
BREthereumAddress primaryAddress = accountGetPrimaryAddress(ewm->account);

assert (ETHEREUM_BOOLEAN_IS_TRUE(addressEqual(primaryAddress, bundle->from))
        || ETHEREUM_BOOLEAN_IS_TRUE(addressEqual(primaryAddress, bundle->to)));

// primaryAddress is either the transaction's `source` or `target`.
BREthereumBoolean isSource = addressEqual(primaryAddress, bundle->from);

BREthereumWalletId wid = ewmGetWallet(ewm);
BREthereumWallet wallet = ewmLookupWallet(ewm, wid);

BREthereumBlock block = ewmLookupBlockByHash(ewm, bundle->blockHash);
block = blockCreateMinimal(bundle->blockHash, bundle->blockNumber, bundle->blockTimestamp);
ewmClientSignalBlockEvent(ewm, ewmInsertBlock(ewm, block),
                          BLOCK_EVENT_CREATED,
                          SUCCESS, NULL);

// Look for a pre-existing transaction
BREthereumTransaction transaction = walletGetTransactionByHash(wallet, bundle->hash);

// If we did not have a transaction for 'hash' it might be (might likely be) a newly submitted
// transaction that we are holding but that doesn't have a hash yet.  This will *only* apply
// if we are the source.
if (NULL == transaction && ETHEREUM_BOOLEAN_IS_TRUE(isSource))
transaction = walletGetTransactionByNonce(wallet, primaryAddress, bundle->nonce);

// If we still don't have a transaction (with 'hash' or 'nonce'); then create one.
if (NULL == transaction) {
    BREthereumAddress sourceAddr = (ETHEREUM_BOOLEAN_IS_TRUE(isSource) ? primaryAddress : bundle->from);
    BREthereumAddress targetAddr = (ETHEREUM_BOOLEAN_IS_TRUE(isSource) ? bundle->to : primaryAddress);

    // Get the amount; this will be '0' if this is a token transfer
    BREthereumAmount amount = amountCreateEther(etherCreate(bundle->amount));

    // Easily extract the gasPrice and gasLimit.
    BREthereumGasPrice gasPrice = gasPriceCreate(etherCreate(bundle->gasPrice));

    BREthereumGas gasLimit = gasCreate(bundle->gasLimit);

    // Finally, get ourselves a transaction.
    transaction = transactionCreate(sourceAddr,
                                    targetAddr,
                                    amount,
                                    gasPrice,
                                    gasLimit,
                                    bundle->nonce);
    // With a new transaction:
    //
    //   a) add to the ewm
    tid = ewmInsertTransaction(ewm, transaction);
    //
    //  b) add to the wallet
    walletHandleTransaction(wallet, transaction);
    //
    //  c) announce the wallet update
    ewmClientSignalTransactionEvent(ewm, wid, tid,
                                    TRANSACTION_EVENT_CREATED,
                                    SUCCESS, NULL);
    //
    //  d) announce as submitted (=> there is a hash, submitted by 'us' or 'them')
    walletTransactionSubmitted(wallet, transaction, bundle->hash);

}

if (-1 == tid)
tid = ewmLookupTransactionId(ewm, transaction);

BREthereumGas gasUsed = gasCreate(bundle->gasUsed);
// TODO: Process 'state' properly - errors?

// Get the current status.
BREthereumTransactionStatus status = transactionGetStatus(transaction);

// Update the status as blocked
if (TRANSACTION_STATUS_INCLUDED != status.type)
walletTransactionIncluded(wallet, transaction, gasUsed,
                          blockGetHash(block),
                          blockGetNumber(block),
                          bundle->blockTransactionIndex);

// Announce a transaction event.  If already 'BLOCKED', then update CONFIRMATIONS.
ewmClientSignalTransactionEvent(ewm, wid, tid,
                                (TRANSACTION_STATUS_INCLUDED == status.type
                                 ? TRANSACTION_EVENT_BLOCK_CONFIRMATIONS_UPDATED
                                 : TRANSACTION_EVENT_BLOCKED),
                                SUCCESS,
                                NULL);
#endif


#if 0 // Log
BREthereumTransactionId tid = -1;

pthread_mutex_lock(&ewm->lock);

// Token of interest
BREthereumToken token = tokenLookupByAddress(bundle->contract);
if (NULL == token) { pthread_mutex_unlock(&ewm->lock); return; } // uninteresting token

// Event of interest
BREthereumContractEvent event = contractLookupEventForTopic (contractERC20, bundle->arrayTopics[0]);
if (NULL == event || event != eventERC20Transfer) { pthread_mutex_unlock(&ewm->lock); return; }; // uninteresting event

BREthereumBlock block = NULL;
//    BREthereumBlock block = ewmLookupBlockByHash(ewm, bundle->blockHash);
//    block = blockCreateMinimal(bundle->blockHash, bundle->blockNumber, bundle->blockTimestamp);
//    ewmClientSignalBlockEvent(ewm, ewmInsertBlock(ewm, block),
//                                BLOCK_EVENT_CREATED,
//                                SUCCESS, NULL);

// Wallet for token
BREthereumWalletId wid = (NULL == token
                          ? ewmGetWallet(ewm)
                          : ewmGetWalletHoldingToken(ewm, token));
BREthereumWallet wallet = ewmLookupWallet(ewm, wid);

// Existing transaction
BREthereumTransaction transaction = walletGetTransactionByHash(wallet, bundle->hash);

BREthereumGasPrice gasPrice = gasPriceCreate(etherCreate(bundle->gasPrice));
BREthereumGas gasUsed = gasCreate(bundle->gasUsed);


// Create a token transaction
if (NULL == transaction) {

    // Parse the topic data - we fake it becasue we 'know' topics indices
    BREthereumAddress sourceAddr =
    addressCreate(eventERC20TransferDecodeAddress(event, bundle->arrayTopics[1]));

    BREthereumAddress targetAddr =
    addressCreate(eventERC20TransferDecodeAddress(event, bundle->arrayTopics[2]));

    BRCoreParseStatus status = CORE_PARSE_OK;

    BREthereumAmount amount =
    amountCreateToken(createTokenQuantity(token, eventERC20TransferDecodeUInt256(event,
                                                                                 bundle->data,
                                                                                 &status)));

    transaction = transactionCreate(sourceAddr, targetAddr, amount, gasPrice, gasUsed, 0);

    // With a new transaction:
    //
    //   a) add to the ewm
    tid = ewmInsertTransaction(ewm, transaction);
    //
    //  b) add to the wallet
    walletHandleTransaction(wallet, transaction);
    //
    //  c) announce the wallet update
    ewmClientSignalTransactionEvent(ewm, wid, tid, TRANSACTION_EVENT_CREATED, SUCCESS, NULL);

    //
    //  d) announce as submitted.
    walletTransactionSubmitted(wallet, transaction, bundle->hash);

}

if (-1 == tid)
tid = ewmLookupTransactionId(ewm, transaction);

// TODO: Process 'state' properly - errors?

// Get the current status.
BREthereumTransactionStatus status = transactionGetStatus(transaction);

// Update the status as blocked
if (TRANSACTION_STATUS_INCLUDED != status.type)
walletTransactionIncluded(wallet, transaction, gasUsed,
                          blockGetHash(block),
                          blockGetNumber(block),
                          bundle->blockTransactionIndex);

// Announce a transaction event.  If already 'BLOCKED', then update CONFIRMATIONS.
ewmClientSignalTransactionEvent(ewm, wid, tid,
                                (TRANSACTION_STATUS_INCLUDED == status.type
                                 ? TRANSACTION_EVENT_BLOCK_CONFIRMATIONS_UPDATED
                                 : TRANSACTION_EVENT_BLOCKED),
                                SUCCESS,
                                NULL);

// Hmmmm...
pthread_mutex_unlock(&ewm->lock);
#endif


#if 0 // token
static int
ewmDataIsEmpty (BREthereumEWM ewm, const char *data) {
    return NULL == data || 0 == strcmp ("", data) || 0 == strcmp ("0x", data);
}

// If `data` is anything besides "0x", then we have a contract function call.  At that point
// it seems we need to process `data` to extract the 'function + args' and then, if the
// function is 'transfer() token' we can then and only then conclude that we have a token

if (ewmDataIsEmpty(ewm, data)) return NULL;

// There is contract data; see if it is a ERC20 function.
BREthereumContractFunction function = contractLookupFunctionForEncoding(contractERC20, data);

// Not an ERC20 token
if (NULL == function) return NULL;

// See if we have an existing token.
BREthereumToken token = tokenLookup(target);
if (NULL == token) token = tokenLookup(contract);

// We found a token...
if (NULL != token) return token;

// ... we didn't find a token - we should create is dynamically.
fprintf (stderr, "Ignoring transaction for unknown ERC20 token at '%s'", target);
return NULL;
#endif


