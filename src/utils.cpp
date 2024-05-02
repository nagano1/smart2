#include <string.h>
#include "common.hpp"
#include "parse_util.hpp"

using letterCheckerType = bool(*)(int, char);


// --------------------------------------------------------------------------
// 
//                               HashMap
// 
// --------------------------------------------------------------------------

int VoidHashMap::calc_hash(const char *key, int keyLength, size_t max)
{
    unsigned int sum = keyLength;
    int border = 128;

    int salt = 0; // prevent same result from only order different of letters
    for (int i = 0; i < keyLength && i < border; i++) {
        unsigned char unsignedValue = key[i];
        sum += unsignedValue;
        salt += i % 2 == 0 ? unsignedValue * i : -unsignedValue * i;
    }

    for (int i = keyLength-1,j=0; i >= border && j < border; i--,j++) {
        unsigned char unsignedValue = key[i];// (key[i] < 0 ? -key[i] : key[i]);
        sum += unsignedValue;
        salt += j % 2 == 0 ? unsignedValue * j : -unsignedValue * j;
    }
    if (salt < 0) {
        salt = -(salt);
    }
    return (sum + salt) % max;
}

void VoidHashMap::put(const char *keyA, int keyLength, void* val) const
{

    auto hashInt = calc_hash(keyA, keyLength, this->entries_length);
    VoidHashNode* hashNode = this->entries[hashInt];

    if (hashNode == nullptr) {// || hashNode->key == nullptr) {
        auto *newHashNode = this->memBuffer->newMem<VoidHashNode>(1);
        newHashNode->next = nullptr;
        this->entries[hashInt] = newHashNode;


        char *keyB = this->memBuffer->newMem<char>(keyLength + 1);
        for (int i = 0; i < keyLength; i++) {
            keyB[i] = keyA[i];
        }
        newHashNode->key = keyB;
        newHashNode->keyLength = keyLength;
        newHashNode->nodeBase = val;
        return;
    }

    while (true) {
        // find same key
        if (hashNode->keyLength == keyLength) {
            bool sameKey = true;
            for (int i = 0; i < keyLength; i++) {
                if (hashNode->key[i] != keyA[i]) {
                    sameKey = false;
                    break;
                }
            }
            if (sameKey) {
                hashNode->nodeBase = val;
                return;
            }
        }

        if (hashNode->next == nullptr) {
            break;
        }
        hashNode = hashNode->next;
    }


    auto *newHashNode = this->memBuffer->newMem<VoidHashNode>(1);
    char *keyB =  this->memBuffer->newMemArray<char>(keyLength + 1);
    for (int i = 0; i < keyLength; i++) {
        keyB[i] = keyA[i];
    }
    newHashNode->key = keyB;
    newHashNode->keyLength = keyLength;
    newHashNode->nodeBase = val;
    newHashNode->next = nullptr;

    hashNode->next = newHashNode;
}


void VoidHashMap::init(MemBuffer* membuffer)
{
    this->memBuffer = membuffer;
    this->entries = (VoidHashNode**)this->memBuffer->newMemArray<VoidHashNode>(HashNode_TABLE_SIZE);
    this->entries_length = HashNode_TABLE_SIZE;
/*
    memset(this->entries, 0, sizeof(VoidHashNode*)*this->entries_length);
    for (unsigned int i = 0; i < this->entries_length; i++) {
        this->entries[i] = nullptr;
    }
    */
}

bool VoidHashMap::has(const char * key, int keyLength)
{
    return this->entries[calc_hash0(key, keyLength)] != nullptr;
}

void VoidHashMap::deleteKey(const char * key, int keyLength)
{
    if (this->entries[calc_hash0(key, keyLength)] != nullptr) {
        free(this->entries[calc_hash0(key, keyLength)]);
    }
}

void* VoidHashMap::get(const char * key, int keyLength)
{
    auto keyInt = calc_hash0(key, keyLength);
    if (this->entries[keyInt] != nullptr) {
        auto * hashNode = this->entries[keyInt];
        while (hashNode) {
            if (hashNode->keyLength == keyLength) {
                bool sameKey = true;
                for (int i = 0; i < keyLength; i++) {
                    if (hashNode->key[i] != key[i]) {
                        sameKey = false;
                        break;
                    }
                }

                if (sameKey) {
                    return hashNode->nodeBase;
                }
            }

            hashNode = hashNode->next;
        }
    }
    return nullptr;
}














// --------------------------------------------------------------------------
// 
//                               ParseUtils
// 
// --------------------------------------------------------------------------

/*
UTF-8

0xxxxxxx                            0 - 127
110yyyyx 10xxxxxx                   128 - 2047
1110yyyy 10yxxxxx 10xxxxxx          2048 - 65535
11110yyy 10yyxxxx 10xxxxxx 10xxxxxx 65536 - 0x10FFFF

at least one of the y should be 1
*/

/*
UTF-16

codepoint 	UTF-16
xxxxxxxxxxxxxxxx         	xxxxxxxxxxxxxxxx
000uuuuuxxxxxxxxxxxxxxxx 	110110wwwwxxxxxx 110111xxxxxxxxxx 	wwww = uuuuu - 1
*/
static constexpr unsigned char utf8BytesTable[256]{
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
        4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
};


/*
*/
static constexpr unsigned int hex_asciicode_table[256]{
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        0,1,2,3,4,5,6,7,8,9,1,1,1,1,1,1, // 48, 48	0x30	0
        1,10,11,12,13,14,15,16,1,1,1,1,1,1,1,1, // 64, , 65	0x41	A
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // 80
        1,10,11,12,13,14,15,16,1,1,1,1,1,1,1,1, // 96, 97	0x61	a
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

    // \u8e60
    int ParseUtil::parseJsonUtf16Sequense(const char* utf16_chars, unsigned int len, int index, int *consumed,
                                      unsigned char* ch1, unsigned char* ch2, unsigned char* ch3, unsigned char* ch4) {

        // \u6382
        if (index + 6 > (int)len) {
            return 0;
        }

        const unsigned char* chars = (unsigned char*)(utf16_chars + index);
        assert(chars[0] == '\\' && chars[1] == 'u');

        unsigned int first8bit = hex_asciicode_table[(int)chars[2]] << 4u | hex_asciicode_table[(int)chars[3]];
        bool hasPair = (first8bit >> 2u) == 0b110110; // 110110ww

        unsigned int codePoint = first8bit << 8u
                        | hex_asciicode_table[(int)chars[4]] << 4u
                        | hex_asciicode_table[(int)chars[5]];

        if (hasPair) {
            // eg. \uD840\uDFF9
            if (index + 12 > (int)len || chars[6] != '\\' || chars[7] != 'u') {
                return 0;
            }

            int bitArray1 = codePoint & 0b0000001111111111; // 110110wwwwxxxxxx
            int bitArray2 = hex_asciicode_table[(int)chars[9]] << 8u
                            | hex_asciicode_table[(int)chars[10]] << 4u
                            | hex_asciicode_table[(int)chars[11]];
            bitArray2 = bitArray2 & 0b0000001111111111; // 110111xxxxxxxxxx

            // utf16 uuuuuxxxxxxxxxxxxxxxx 	110110wwww_xxxx_xx 110111xxxx_xxxxxx 	(wwww = uuuuu - 1)
            // utf8 11110yyy 10yyxxxx 10xxxxxx 10xxxxxx 65536 - 0x10FFFF
            int u5bit = (bitArray1 >> 6u) + 1;
            *ch1 = u5bit >> 3u | 0b11110000;
            *ch2 = (u5bit & 0b11) << 4u | ((bitArray1 >> 2u) & 0b1111) | 0x80;
            *ch3 = (bitArray1 & 0b11) << 4u | bitArray2 >> 6u | 0x80;
            *ch4 = (bitArray2 & 0b111111) | 0x80;
            *consumed = 12;
            return 4;
        }
        else {
            *consumed = 6;

            if (codePoint <= 127) { // 0xxxxxxx 0 - 127
                *ch1 = codePoint;
                return 1;
            } else if (codePoint <= 2047) { // 110yyyyx 10xxxxxx 128 - 2047
                *ch1 = codePoint>>6 | 0xC0;
                *ch2 = (codePoint & 0x3F) | 0x80;
                return 2;
            } else if (codePoint <= 65535) { // 1110yyyy 10yxxxxx 10xxxxxx 2048 - 65535
                *ch1 = codePoint >> 12u | 0b11100000;
                *ch2 = (codePoint & 0b111111000000) >> 6u | 0x80;
                *ch3 = (codePoint & 0b111111) | 0x80;
                return 3;
            }
        }

        return 0;
    }



    int ParseUtil::utf16_length(const char *utf8_chars, unsigned int byte_len) {
        unsigned int pos = 0;
        int length = 0;

        while (pos < byte_len) {
            auto idx = (unsigned char)utf8_chars[pos];
            int bytes = utf8BytesTable[idx];
            pos += bytes;
            length += bytes > 3 ? 2 : 1;
        }
        return length;
    }





    // EXPECT_EQ(0, Tokenizer::matchAt("class A{}", "class"));
    int ParseUtil::_matchFirstWithTrim(const char *chars, int charsLength, const char *target, int start)
    {
        //  return -1 if it fails
        int currentTargetIndex = 0;
        int matchStartIndex = -1;

        for (int i = start; true; i++) {
            auto ch = chars[i];

            if ((ch & 0x80) != 0x80) {
                if (ch == '\0') {
                    return -1;
                }

                if (matchStartIndex == -1) {
                    if (ch == ' ') { //allow trim
                        continue;
                    }
                    else if (ch == '\t' || ch == '\n' || ch == '\r') {
                        continue;
                    }
                    else { // should start match
                        matchStartIndex = i;
                        currentTargetIndex = 0;
                    }
                }

                if (target[currentTargetIndex] == '\0') {
                    break;
                }

                if (target[currentTargetIndex] == chars[i]) {
                    currentTargetIndex++;
                    continue;
                }
            }

            return -1;
        }

        if (currentTargetIndex == 0) {
            return -1;
        }
        else {
            return matchStartIndex;
        }
    };


