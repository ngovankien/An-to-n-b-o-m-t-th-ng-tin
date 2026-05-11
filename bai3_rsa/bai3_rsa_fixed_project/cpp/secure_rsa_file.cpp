#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/core_names.h>
#include <openssl/param_build.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

using Bytes = std::vector<unsigned char>;

static const std::string MAGIC = "BT3RSA01";

void printOpenSSLError(const std::string& msg) {
    std::cerr << msg << "\n";
    ERR_print_errors_fp(stderr);
}

Bytes readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Khong mo duoc file: " + path);
    return Bytes((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

void writeFile(const std::string& path, const Bytes& data) {
    std::ofstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Khong ghi duoc file: " + path);
    f.write(reinterpret_cast<const char*>(data.data()), (std::streamsize)data.size());
}

void writeText(const std::string& path, const std::string& data) {
    std::ofstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Khong ghi duoc file: " + path);
    f << data;
}

std::string readText(const std::string& path) {
    auto b = readFile(path);
    return std::string(b.begin(), b.end());
}

void putU32(Bytes& out, uint32_t v) {
    out.push_back((v >> 24) & 0xff);
    out.push_back((v >> 16) & 0xff);
    out.push_back((v >> 8) & 0xff);
    out.push_back(v & 0xff);
}

uint32_t getU32(const Bytes& in, size_t& off) {
    if (off + 4 > in.size()) throw std::runtime_error("Package loi: thieu do dai");
    uint32_t v = ((uint32_t)in[off] << 24) | ((uint32_t)in[off+1] << 16) | ((uint32_t)in[off+2] << 8) | (uint32_t)in[off+3];
    off += 4;
    return v;
}

void putField(Bytes& out, const Bytes& field) {
    if (field.size() > 0xffffffffULL) throw std::runtime_error("Field qua lon");
    putU32(out, (uint32_t)field.size());
    out.insert(out.end(), field.begin(), field.end());
}

Bytes getField(const Bytes& in, size_t& off) {
    uint32_t len = getU32(in, off);
    if (off + len > in.size()) throw std::runtime_error("Package loi: field vuot qua kich thuoc");
    Bytes b(in.begin() + off, in.begin() + off + len);
    off += len;
    return b;
}

EVP_PKEY* readPrivateKey(const std::string& path) {
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) throw std::runtime_error("Khong mo duoc private key: " + path);
    EVP_PKEY* key = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
    fclose(fp);
    if (!key) throw std::runtime_error("Khong doc duoc private key PEM: " + path);
    return key;
}

EVP_PKEY* readPublicKey(const std::string& path) {
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) throw std::runtime_error("Khong mo duoc public key: " + path);
    EVP_PKEY* key = PEM_read_PUBKEY(fp, nullptr, nullptr, nullptr);
    fclose(fp);
    if (!key) throw std::runtime_error("Khong doc duoc public key PEM: " + path);
    return key;
}

void writePrivateKey(const std::string& path, EVP_PKEY* key) {
    FILE* fp = fopen(path.c_str(), "wb");
    if (!fp) throw std::runtime_error("Khong ghi duoc private key: " + path);
    int ok = PEM_write_PrivateKey(fp, key, nullptr, nullptr, 0, nullptr, nullptr);
    fclose(fp);
    if (!ok) throw std::runtime_error("Loi ghi private key PEM");
}

void writePublicKey(const std::string& path, EVP_PKEY* key) {
    FILE* fp = fopen(path.c_str(), "wb");
    if (!fp) throw std::runtime_error("Khong ghi duoc public key: " + path);
    int ok = PEM_write_PUBKEY(fp, key);
    fclose(fp);
    if (!ok) throw std::runtime_error("Loi ghi public key PEM");
}

int generateKeys(const std::string& prefix) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) throw std::runtime_error("Khong tao duoc RSA context");
    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen_init(ctx) <= 0) throw std::runtime_error("Loi khoi tao keygen");
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) throw std::runtime_error("Loi dat RSA 2048 bit");
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) throw std::runtime_error("Loi tao khoa RSA");
    writePrivateKey(prefix + "_private.pem", pkey);
    writePublicKey(prefix + "_public.pem", pkey);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    std::cout << "OK: da tao " << prefix << "_private.pem va " << prefix << "_public.pem\n";
    return 0;
}

Bytes aesGcmEncrypt(const Bytes& plaintext, const Bytes& key, const Bytes& iv, Bytes& tag) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Khong tao AES ctx");
    Bytes ciphertext(plaintext.size() + 16);
    int len = 0, total = 0;
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) throw std::runtime_error("AES init loi");
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)iv.size(), nullptr) != 1) throw std::runtime_error("AES IV len loi");
    if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), iv.data()) != 1) throw std::runtime_error("AES set key loi");
    if (!plaintext.empty() && EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), (int)plaintext.size()) != 1) throw std::runtime_error("AES encrypt loi");
    total = len;
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + total, &len) != 1) throw std::runtime_error("AES final loi");
    total += len;
    ciphertext.resize(total);
    tag.resize(16);
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1) throw std::runtime_error("AES get tag loi");
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

Bytes aesGcmDecrypt(const Bytes& ciphertext, const Bytes& key, const Bytes& iv, const Bytes& tag) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Khong tao AES ctx");
    Bytes plaintext(ciphertext.size() + 16);
    int len = 0, total = 0;
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) throw std::runtime_error("AES init loi");
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)iv.size(), nullptr) != 1) throw std::runtime_error("AES IV len loi");
    if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), iv.data()) != 1) throw std::runtime_error("AES set key loi");
    if (!ciphertext.empty() && EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), (int)ciphertext.size()) != 1) throw std::runtime_error("AES decrypt loi");
    total = len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, (int)tag.size(), (void*)tag.data()) != 1) throw std::runtime_error("AES set tag loi");
    int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + total, &len);
    EVP_CIPHER_CTX_free(ctx);
    if (ret != 1) throw std::runtime_error("Giai ma that bai: file bi sua hoac khoa nhan khong dung");
    total += len;
    plaintext.resize(total);
    return plaintext;
}

Bytes rsaEncryptOAEP(EVP_PKEY* pub, const Bytes& data) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pub, nullptr);
    if (!ctx) throw std::runtime_error("RSA encrypt ctx loi");
    if (EVP_PKEY_encrypt_init(ctx) <= 0) throw std::runtime_error("RSA encrypt init loi");
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) throw std::runtime_error("RSA OAEP padding loi");
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) throw std::runtime_error("RSA OAEP SHA256 loi");
    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, data.data(), data.size()) <= 0) throw std::runtime_error("RSA encrypt size loi");
    Bytes out(outlen);
    if (EVP_PKEY_encrypt(ctx, out.data(), &outlen, data.data(), data.size()) <= 0) throw std::runtime_error("RSA encrypt loi");
    out.resize(outlen);
    EVP_PKEY_CTX_free(ctx);
    return out;
}

Bytes rsaDecryptOAEP(EVP_PKEY* priv, const Bytes& data) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(priv, nullptr);
    if (!ctx) throw std::runtime_error("RSA decrypt ctx loi");
    if (EVP_PKEY_decrypt_init(ctx) <= 0) throw std::runtime_error("RSA decrypt init loi");
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) throw std::runtime_error("RSA OAEP padding loi");
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) throw std::runtime_error("RSA OAEP SHA256 loi");
    size_t outlen = 0;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, data.data(), data.size()) <= 0) throw std::runtime_error("RSA decrypt size loi");
    Bytes out(outlen);
    if (EVP_PKEY_decrypt(ctx, out.data(), &outlen, data.data(), data.size()) <= 0) throw std::runtime_error("RSA decrypt loi: khoa bi mat may nhan khong dung");
    out.resize(outlen);
    EVP_PKEY_CTX_free(ctx);
    return out;
}

Bytes signPSS(EVP_PKEY* priv, const Bytes& data) {
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) throw std::runtime_error("Sign ctx loi");
    EVP_PKEY_CTX* pctx = nullptr;
    if (EVP_DigestSignInit(mdctx, &pctx, EVP_sha256(), nullptr, priv) <= 0) throw std::runtime_error("Sign init loi");
    if (EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PSS_PADDING) <= 0) throw std::runtime_error("PSS padding loi");
    if (EVP_PKEY_CTX_set_rsa_pss_saltlen(pctx, -1) <= 0) throw std::runtime_error("PSS salt loi");
    if (EVP_DigestSignUpdate(mdctx, data.data(), data.size()) <= 0) throw std::runtime_error("Sign update loi");
    size_t siglen = 0;
    if (EVP_DigestSignFinal(mdctx, nullptr, &siglen) <= 0) throw std::runtime_error("Sign size loi");
    Bytes sig(siglen);
    if (EVP_DigestSignFinal(mdctx, sig.data(), &siglen) <= 0) throw std::runtime_error("Sign final loi");
    sig.resize(siglen);
    EVP_MD_CTX_free(mdctx);
    return sig;
}

bool verifyPSS(EVP_PKEY* pub, const Bytes& data, const Bytes& sig) {
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) throw std::runtime_error("Verify ctx loi");
    EVP_PKEY_CTX* pctx = nullptr;
    if (EVP_DigestVerifyInit(mdctx, &pctx, EVP_sha256(), nullptr, pub) <= 0) throw std::runtime_error("Verify init loi");
    if (EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PSS_PADDING) <= 0) throw std::runtime_error("PSS padding loi");
    if (EVP_PKEY_CTX_set_rsa_pss_saltlen(pctx, -1) <= 0) throw std::runtime_error("PSS salt loi");
    if (EVP_DigestVerifyUpdate(mdctx, data.data(), data.size()) <= 0) throw std::runtime_error("Verify update loi");
    int ok = EVP_DigestVerifyFinal(mdctx, sig.data(), sig.size());
    EVP_MD_CTX_free(mdctx);
    return ok == 1;
}

Bytes makePackageBody(const std::string& filename, const Bytes& encKey, const Bytes& iv, const Bytes& tag, const Bytes& ciphertext) {
    Bytes body;
    body.insert(body.end(), MAGIC.begin(), MAGIC.end());
    putField(body, Bytes(filename.begin(), filename.end()));
    putField(body, encKey);
    putField(body, iv);
    putField(body, tag);
    putField(body, ciphertext);
    return body;
}

struct Package {
    std::string filename;
    Bytes encKey, iv, tag, ciphertext, signature;
    Bytes signedBody;
};

Package parsePackage(const Bytes& pkg) {
    Package p;
    size_t off = 0;
    if (pkg.size() < MAGIC.size()) throw std::runtime_error("Package khong hop le");
    std::string magic((const char*)pkg.data(), MAGIC.size());
    if (magic != MAGIC) throw std::runtime_error("Package khong dung dinh dang BT3RSA01");
    off = MAGIC.size();
    Bytes name = getField(pkg, off);
    p.filename = std::string(name.begin(), name.end());
    p.encKey = getField(pkg, off);
    p.iv = getField(pkg, off);
    p.tag = getField(pkg, off);
    p.ciphertext = getField(pkg, off);
    p.signedBody = Bytes(pkg.begin(), pkg.begin() + off);
    p.signature = getField(pkg, off);
    if (off != pkg.size()) throw std::runtime_error("Package loi: du du lieu cuoi file");
    return p;
}

std::string safeBaseName(const std::string& name) {
    std::filesystem::path p(name);
    std::string base = p.filename().string();
    if (base.empty()) base = "decrypted_file";
    return base;
}

int packFile(const std::string& inputFile, const std::string& receiverPubPath, const std::string& senderPrivPath, const std::string& outPkg) {
    Bytes plaintext = readFile(inputFile);
    Bytes aesKey(32), iv(12), tag;
    if (RAND_bytes(aesKey.data(), (int)aesKey.size()) != 1 || RAND_bytes(iv.data(), (int)iv.size()) != 1) throw std::runtime_error("RAND_bytes loi");

    EVP_PKEY* receiverPub = readPublicKey(receiverPubPath);
    EVP_PKEY* senderPriv = readPrivateKey(senderPrivPath);
    Bytes encKey = rsaEncryptOAEP(receiverPub, aesKey);
    Bytes ciphertext = aesGcmEncrypt(plaintext, aesKey, iv, tag);
    std::string fname = std::filesystem::path(inputFile).filename().string();
    Bytes body = makePackageBody(fname, encKey, iv, tag, ciphertext);
    Bytes sig = signPSS(senderPriv, body);
    putField(body, sig);
    writeFile(outPkg, body);
    EVP_PKEY_free(receiverPub);
    EVP_PKEY_free(senderPriv);
    std::cout << "OK: da ma hoa + ky file. Goi gui: " << outPkg << "\n";
    return 0;
}

int unpackFile(const std::string& pkgPath, const std::string& receiverPrivPath, const std::string& senderPubPath, const std::string& outDir) {
    Bytes pkg = readFile(pkgPath);
    Package p = parsePackage(pkg);
    EVP_PKEY* senderPub = readPublicKey(senderPubPath);
    bool ok = verifyPSS(senderPub, p.signedBody, p.signature);
    EVP_PKEY_free(senderPub);
    if (!ok) throw std::runtime_error("Chu ky KHONG hop le: khong xac thuc duoc nguoi gui hoac goi da bi sua");

    EVP_PKEY* receiverPriv = readPrivateKey(receiverPrivPath);
    Bytes aesKey = rsaDecryptOAEP(receiverPriv, p.encKey);
    EVP_PKEY_free(receiverPriv);
    Bytes plaintext = aesGcmDecrypt(p.ciphertext, aesKey, p.iv, p.tag);

    std::filesystem::create_directories(outDir);
    std::string outPath = (std::filesystem::path(outDir) / ("decrypted_" + safeBaseName(p.filename))).string();
    writeFile(outPath, plaintext);
    std::cout << "OK: chu ky hop le. Da giai ma file: " << outPath << "\n";
    return 0;
}

void usage() {
    std::cerr << "BT3 RSA Secure File\n"
              << "  gen <prefix>\n"
              << "  pack <input_file> <receiver_public.pem> <sender_private.pem> <out.bt3>\n"
              << "  unpack <package.bt3> <receiver_private.pem> <sender_public.pem> <out_dir>\n";
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) { usage(); return 1; }
        std::string cmd = argv[1];
        if (cmd == "gen" && argc == 3) return generateKeys(argv[2]);
        if (cmd == "pack" && argc == 6) return packFile(argv[2], argv[3], argv[4], argv[5]);
        if (cmd == "unpack" && argc == 6) return unpackFile(argv[2], argv[3], argv[4], argv[5]);
        usage();
        return 1;
    } catch (const std::exception& e) {
        printOpenSSLError(std::string("LOI: ") + e.what());
        return 2;
    }
}
