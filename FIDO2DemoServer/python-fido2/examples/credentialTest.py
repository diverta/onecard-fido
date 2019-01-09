#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function, absolute_import, unicode_literals

from fido2.hid import CtapHidDevice
from fido2.client import Fido2Client
from getpass import getpass
import sys
import binascii

def setupDevice():
    '''
    デバイスセットアップ
    '''
    # Locate a device
    dev = next(CtapHidDevice.list_devices(), None)
    if not dev:
        print('No FIDO device found')
        sys.exit(1)
    
    # Set up a FIDO 2 client using the origin https://example.com
    client = Fido2Client(dev, 'https://example.com')
    return client

# Create a credential
def makeCredential(rp, user, challenge):
    print('\nTouch your authenticator device now...\n')
    '''
    try:
        attestation_object, client_data = client.make_credential(
            rp, user, challenge)
    except ValueError:
        attestation_object, client_data = client.make_credential(
            rp, user, challenge,
            pin=getpass('Please enter PIN:'))
    '''
    attestation_object, client_data = client.make_credential(
        rp, user, challenge)
    
    
    print('New credential created!')
    
    print('CLIENT DATA:', client_data)
    print('ATTESTATION OBJECT:', attestation_object)
    print()
    print('CREDENTIAL DATA:', attestation_object.auth_data.credential_data)
    
    # Verify signature
    attestation_object.verify(client_data.hash)
    print('Attestation signature verified!')
    
    return attestation_object.auth_data.credential_data


# Authenticate the credential
def getAssertion(rp, challenge, allowList, credential):
    print('\nTouch your authenticator device now...\n')
    
    try:
        assertions, client_data = client.get_assertion(
            rp['id'], challenge, allowList)
    except ValueError:
        assertions, client_data = client.get_assertion(
            rp['id'], challenge, allowList,
            pin=getpass('Please enter PIN:'))
    
    print('Credential authenticated!')
    
    # Only one cred in allowList, only one response.
    assertion = assertions[0]
    
    print('CLIENT DATA:', client_data)
    print()
    print('ASSERTION DATA:', assertion)
    
    # Verify signature
    assertion.verify(client_data.hash, credential.public_key)
    print('Assertion signature verified!')


'''
テストの実行
'''
# デバイスセットアップ
client = setupDevice()

# RP（サイト）は１通り生成
rp = {'id':'example.com', 'name':'Example RP'}

# ユーザーIDを２通り生成
userIdA = binascii.unhexlify(b'49505152535456525354555656506566')
userIdB = binascii.unhexlify(b'54565354524965665051556552535650')

'''
makeCredentialを２ユーザー個別に実行
'''
# makeCredentialパラメーターを個別に生成
userA = {'id':userIdA, 'name':'User A'}
userB = {'id':userIdB, 'name':'User B'}

# makeCredential実行
credentialA = makeCredential(rp, userA, 'Y2hhbGxlbmdl')
credentialB = makeCredential(rp, userB, 'Q3giaLuodnye')

'''
getAssertionを２ユーザー個別に実行（Aユーザー:2回、Bユーザー:3回）
テスト完了後、署名カウンターが以下のようになっていることを確認
 Aユーザー = 2
 Bユーザー = 3
'''
# credential IDはユーザー個別に指定
allowList_A = [{'type':'public-key', 'id':credentialA.credential_id}]
allowList_B = [{'type':'public-key', 'id':credentialB.credential_id}]

# getAssertion実行
getAssertion(rp, 'Q0hBTExFTkdF', allowList_A, credentialA)
getAssertion(rp, 'W9jNYRuHRjxG', allowList_B, credentialB)
getAssertion(rp, 'W9jNYRuHRjxG', allowList_B, credentialB)
getAssertion(rp, 'W9jNYRuHRjxG', allowList_B, credentialB)
getAssertion(rp, 'Q0hBTExFTkdF', allowList_A, credentialA)
