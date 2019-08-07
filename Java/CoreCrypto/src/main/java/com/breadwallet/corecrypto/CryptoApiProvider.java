/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 5/31/18.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corecrypto;

import com.breadwallet.crypto.CryptoApi;
import com.breadwallet.crypto.Unit;
import com.breadwallet.crypto.blockchaindb.BlockchainDb;
import com.breadwallet.crypto.events.system.SystemListener;
import com.google.common.base.Optional;

import java.util.Date;
import java.util.List;
import java.util.concurrent.ExecutorService;

public final class CryptoApiProvider implements CryptoApi.Provider {

    private static final CryptoApiProvider INSTANCE = new CryptoApiProvider();

    public static CryptoApiProvider getInstance() {
        return INSTANCE;
    }

    private static final CryptoApi.AccountProvider accountProvider = new CryptoApi.AccountProvider() {
        @Override
        public com.breadwallet.crypto.Account createFromPhrase(byte[] phraseUtf8, Date timestamp, String uids) {
            return Account.createFromPhrase(phraseUtf8, timestamp, uids);
        }

        @Override
        public Optional<com.breadwallet.crypto.Account> createFromSerialization(byte[] serialization, String uids) {
            return Account.createFromSerialization(serialization, uids).transform(a -> a);
        }

        @Override
        public byte[] generatePhrase(List<String> words) {
            return Account.generatePhrase(words);
        }

        @Override
        public boolean validatePhrase(byte[] phraseUtf8, List<String> words) {
            return Account.validatePhrase(phraseUtf8, words);
        }
    };

    private static final CryptoApi.AmountProvider amountProvider = new CryptoApi.AmountProvider() {
        @Override
        public Optional<com.breadwallet.crypto.Amount> create(long value, Unit unit) {
            return Amount.create(value, unit).transform(a -> a);
        }

        @Override
        public Optional<com.breadwallet.crypto.Amount> create(double value, Unit unit) {
            return Amount.create(value, unit).transform(a -> a);
        }

        @Override
        public Optional<com.breadwallet.crypto.Amount> create(String value, boolean isNegative, Unit unit) {
            return Amount.create(value, isNegative, unit).transform(a -> a);
        }
    };

    private static final CryptoApi.SystemProvider systemProvider = new CryptoApi.SystemProvider() {
        @Override
        public com.breadwallet.crypto.System create(ExecutorService listenerExecutor,
                                                    SystemListener listener,
                                                    com.breadwallet.crypto.Account account,
                                                    boolean isMainnet,
                                                    String path,
                                                    BlockchainDb query) {
            return System.create(listenerExecutor, listener, account, isMainnet, path, query);
        }
    };

    private static final CryptoApi.CoderProvider coderProvider = new CryptoApi.CoderProvider() {
        @Override
        public Coder createCoderForAlgorithm(Coder.Algorithm algorithm) {
            return Coder.createForAlgorithm(algorithm);
        }
    };

    private static final CryptoApi.EncrypterProvider encrypterProvider = new CryptoApi.EncrypterProvider() {
        @Override
        public Encrypter createEncrypterrForAesEcb(byte[] key) {
            return Encrypter.createForAesEcb(key);
        }

        @Override
        public Encrypter createEncrypterForChaCha20Poly1305(com.breadwallet.crypto.Key key, byte[] nonce12, byte[] ad) {
            return Encrypter.createForChaCha20Poly1305(key, nonce12, ad);
        }

        @Override
        public Encrypter createEncrypterForPigeon(com.breadwallet.crypto.Key privKey,
                                                  com.breadwallet.crypto.Key pubKey, byte[] nonce12) {
            return Encrypter.createForPigeon(privKey, pubKey, nonce12);
        }
    };

    private static final CryptoApi.HasherProvider hasherProvider = new CryptoApi.HasherProvider() {
        @Override
        public Hasher createHasherForAlgorithm(Hasher.Algorithm algorithm) {
            return Hasher.createForAlgorithm(algorithm);
        }
    };

    private static final CryptoApi.KeyProvider keyProvider = new CryptoApi.KeyProvider() {
        @Override
        public Optional<com.breadwallet.crypto.Key> createFromPhrase(byte[] phraseUtf8, List<String> words) {
            return Key.createFromPhrase(phraseUtf8, words).transform(a -> a);
        }

        @Override
        public Optional<com.breadwallet.crypto.Key> createFromPrivateKeyString(byte[] keyStringUtf8) {
            return Key.createFromPrivateKeyString(keyStringUtf8).transform(a -> a);
        }

        @Override
        public Optional<com.breadwallet.crypto.Key> createFromPublicKeyString(byte[] keyStringUtf8) {
            return Key.createFromPublicKeyString(keyStringUtf8).transform(a -> a);
        }

        public Optional<com.breadwallet.crypto.Key> createForPigeon(com.breadwallet.crypto.Key key, byte[] nonce) {
            return Key.createForPigeon(key, nonce).transform(a -> a);
        }

        @Override
        public Optional<com.breadwallet.crypto.Key> createForBIP32ApiAuth(byte[] phraseUtf8, List<String> words) {
            return Key.createForBIP32ApiAuth(phraseUtf8, words).transform(a -> a);
        }

        @Override
        public Optional<com.breadwallet.crypto.Key> createForBIP32BitID(byte[] phraseUtf8, int index, String uri, List<String> words) {
            return Key.createForBIP32BitID(phraseUtf8, index, uri, words).transform(a -> a);
        }
    };

    private static final CryptoApi.SignerProvider signerProvider = new CryptoApi.SignerProvider() {
        @Override
        public Signer createSignerForAlgorithm(Signer.Algorithm algorithm) {
            return Signer.createForAlgorithm(algorithm);
        }
    };

    private CryptoApiProvider() {

    }

    @Override
    public CryptoApi.AccountProvider accountProvider() {
        return accountProvider;
    }

    @Override
    public CryptoApi.AmountProvider amountProvider() {
        return amountProvider;
    }

    @Override
    public CryptoApi.SystemProvider systemProvider() {
        return systemProvider;
    }

    @Override
    public CryptoApi.CoderProvider coderPrivider() {
        return coderProvider;
    }

    @Override
    public CryptoApi.EncrypterProvider encrypterProvider() {
        return encrypterProvider;
    }

    @Override
    public CryptoApi.HasherProvider hasherProvider() {
        return hasherProvider;
    }

    @Override
    public CryptoApi.KeyProvider keyProvider() {
        return keyProvider;
    }

    @Override
    public CryptoApi.SignerProvider signerProvider() {
        return signerProvider;
    }
}
