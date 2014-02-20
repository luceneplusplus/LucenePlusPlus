/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "Base64.h"

using namespace Lucene;

typedef LuceneTestFixture Base64Test;

TEST_F(Base64Test, testEncodeSmall) {
    SingleString testBinary = "this is test binary";
    String encode = Base64::encode((uint8_t*)testBinary.c_str(), testBinary.length());
    EXPECT_EQ(encode, L"dGhpcyBpcyB0ZXN0IGJpbmFyeQ==");
}

TEST_F(Base64Test, testEncodeLarge) {
    SingleString testBinary = "This is a larger test string that should convert into base64 "
                              "This is a larger test string that should convert into base64 "
                              "This is a larger test string that should convert into base64 "
                              "This is a larger test string that should convert into base64 "
                              "This is a larger test string that should convert into base64";
    String encode = Base64::encode((uint8_t*)testBinary.c_str(), testBinary.length());

    String expected = L"VGhpcyBpcyBhIGxhcmdlciB0ZXN0IHN0cmluZyB0aGF0IHNob3VsZCBjb252ZXJ0IGludG8gYmFz"
                      L"ZTY0IFRoaXMgaXMgYSBsYXJnZXIgdGVzdCBzdHJpbmcgdGhhdCBzaG91bGQgY29udmVydCBpbnRv"
                      L"IGJhc2U2NCBUaGlzIGlzIGEgbGFyZ2VyIHRlc3Qgc3RyaW5nIHRoYXQgc2hvdWxkIGNvbnZlcnQg"
                      L"aW50byBiYXNlNjQgVGhpcyBpcyBhIGxhcmdlciB0ZXN0IHN0cmluZyB0aGF0IHNob3VsZCBjb252"
                      L"ZXJ0IGludG8gYmFzZTY0IFRoaXMgaXMgYSBsYXJnZXIgdGVzdCBzdHJpbmcgdGhhdCBzaG91bGQg"
                      L"Y29udmVydCBpbnRvIGJhc2U2NA==";

    EXPECT_EQ(encode, expected);
}

TEST_F(Base64Test, testDecodeSmall) {
    String testString = L"dGhpcyBpcyB0ZXN0IGJpbmFyeQ==";
    ByteArray decode = Base64::decode(testString);
    SingleString decodeBinary((char*)decode.get(), decode.size());
    EXPECT_EQ(decodeBinary, "this is test binary");
}

TEST_F(Base64Test, testDecodeLaarge) {
    String testString = L"VGhpcyBpcyBhIGxhcmdlciB0ZXN0IHN0cmluZyB0aGF0IHNob3VsZCBjb252ZXJ0IGludG8gYmFz"
                        L"ZTY0IFRoaXMgaXMgYSBsYXJnZXIgdGVzdCBzdHJpbmcgdGhhdCBzaG91bGQgY29udmVydCBpbnRv"
                        L"IGJhc2U2NCBUaGlzIGlzIGEgbGFyZ2VyIHRlc3Qgc3RyaW5nIHRoYXQgc2hvdWxkIGNvbnZlcnQg"
                        L"aW50byBiYXNlNjQgVGhpcyBpcyBhIGxhcmdlciB0ZXN0IHN0cmluZyB0aGF0IHNob3VsZCBjb252"
                        L"ZXJ0IGludG8gYmFzZTY0IFRoaXMgaXMgYSBsYXJnZXIgdGVzdCBzdHJpbmcgdGhhdCBzaG91bGQg"
                        L"Y29udmVydCBpbnRvIGJhc2U2NA==";
    ByteArray decode = Base64::decode(testString);
    SingleString decodeBinary((char*)decode.get(), decode.size());

    SingleString expected = "This is a larger test string that should convert into base64 "
                            "This is a larger test string that should convert into base64 "
                            "This is a larger test string that should convert into base64 "
                            "This is a larger test string that should convert into base64 "
                            "This is a larger test string that should convert into base64";

    EXPECT_EQ(decodeBinary, expected);
}
