'''
test registerdid/modifydid when fork occured
'''
from utils import common
from TestCase.MVSTestCase import *

class TestFork(ForkTestCase):
    def test_0_fork_at_send(self):
        self.make_partion()
        # make transaction and mine
        Alice.send_etp(Bob.mainaddress(), 10**8)
        Alice.mining()

        self.fork()

        # check if Alice & Bob's address_did record is cleaned
        for role in [Alice, Bob]:
            ec, message = mvs_rpc.list_dids(role.name, role.password)
            self.assertEqual(ec, 0, message)

            self.assertIn({'status':"registered",
            'address':role.mainaddress(),
            'symbol':role.did_symbol}, message['dids'])

    def test_1_fork_at_registerdid(self):
        self.make_partion()
        try:
            target_address = Alice.addresslist[-1]
            # registerdid and mine
            did_symbol = "YouShouldNotSeeThis.Avatar"
            Alice.send_etp(target_address, 10 ** 8)
            Alice.mining()

            ec, message = Alice.register_did(target_address, did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            ec, message = mvs_rpc.list_dids(Alice.name, Alice.password)
            self.assertEqual(ec, 0, message)

            self.assertIn({'status': "registered",
                           'address': target_address,
                           'symbol': did_symbol}, message['dids'])
        finally:
            self.fork()

        ec, message = mvs_rpc.list_dids(Alice.name, Alice.password)
        self.assertEqual(ec, 0, message)
        self.assertNotIn({'status': "registered",
                       'address': target_address,
                       'symbol': Alice.did_symbol}, message['dids'])

    def test_2_fork_at_issueasset(self):
        self.make_partion()

        asset_symbol = None
        domain_symbol = None
        try:
            domain = (u'Not1Exist' + common.get_random_str()).upper()
            domain_symbol, asset_symbol = Alice.create_random_asset(domain_symbol=domain)
            Alice.mining()

            # check asset
            ec, message = mvs_rpc.get_asset( )
            self.assertEqual(ec, 0, message)
            self.assertIn(asset_symbol, message)

            addressassets = Alice.get_addressasset(Alice.mainaddress())
            addressasset = list( filter(lambda a: a.symbol == asset_symbol, addressassets) )
            self.assertEqual(len(addressasset), 1)

            # check domain cert
            certs = Alice.get_addressasset(Alice.mainaddress(), True)
            cert = list( filter(lambda a: a.symbol == domain_symbol, certs) )
            self.assertEqual(len(cert), 1)

        finally:
            self.fork()

        # check asset
        ec, message = mvs_rpc.get_asset( )
        self.assertEqual(ec, 0, message)
        self.assertNotIn(asset_symbol, message)

        addressassets = Alice.get_addressasset(Alice.mainaddress())
        addressasset = list( filter(lambda a: a.symbol == asset_symbol, addressassets) )
        self.assertEqual(len(addressasset), 0)

        # check domain cert
        certs = Alice.get_addressasset(Alice.mainaddress(), True)
        cert = list( filter(lambda a: a.symbol == domain_symbol, certs) )
        self.assertEqual(len(cert), 0)

    def test_3_fork_at_change_did(self):
        self.make_partion()
        try:
            target_addr = Cindy.addresslist[-1]
            Alice.send_etp(target_addr, 10**8)
            Alice.mining()

            ec, message = mvs_rpc.change_did(Cindy.name, Cindy.password, target_addr, Cindy.did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            expect = {
                u'status': u'registered',
                u'symbol': Cindy.did_symbol,
                u'address': target_addr
            }

            ec, message = mvs_rpc.list_dids(Cindy.name, Cindy.password)
            self.assertEqual(ec, 0, message)
            self.assertIn(expect, message['dids'])

            ec, message = mvs_rpc.list_dids()
            self.assertEqual(ec, 0, message)
            self.assertIn(expect, message['dids'])


            target_addr = Cindy.addresslist[-2]
            Alice.send_etp(target_addr, 10**8)
            Alice.mining()

            ec, message = mvs_rpc.change_did(Cindy.name, Cindy.password, target_addr, Cindy.did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            expect = {
                u'status': u'registered',
                u'symbol': Cindy.did_symbol,
                u'address': target_addr
            }

            ec, message = mvs_rpc.list_dids(Cindy.name, Cindy.password)
            self.assertEqual(ec, 0, message)
            self.assertIn(expect, message['dids'])


            ec, message = mvs_rpc.list_dids()
            self.assertEqual(ec, 0, message)
            self.assertIn(expect, message['dids'])

        finally:
            self.fork()

        expect = {
            u'status': u'registered',
            u'symbol': Cindy.did_symbol,
            u'address': Cindy.mainaddress()
        }

        ec, message = mvs_rpc.list_dids(Cindy.name, Cindy.password)
        self.assertEqual(ec, 0, message)
        self.assertIn(expect, message['dids'])

        ec, message = mvs_rpc.list_dids()
        self.assertEqual(ec, 0, message)
        self.assertIn(expect, message['dids'])

    def test_4_fork_at_issuecert(self):
        self.make_partion()

        cert_symbol = None
        try:
            domain = (u'Not2Exist' + common.get_random_str()).upper()
            domain_symbol, asset_symbol = Alice.create_random_asset(domain_symbol=domain)
            Alice.mining()

            cert_symbol = (domain_symbol + ".NAMING").upper();
            ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Alice.did_symbol, cert_symbol, "NAMING")
            self.assertEqual(ec, 0, message)
            Alice.mining()

            # check naming cert
            certs = Alice.get_addressasset(Alice.didaddress(), True)


            cert = list( filter(lambda a: a.symbol == cert_symbol, certs) )
            self.assertEqual(len(cert), 1)

        finally:
            self.fork()

        # check cert
        certs = Alice.get_addressasset(Alice.didaddress(), True)
        cert = list( filter(lambda a: a.symbol == cert_symbol, certs) )
        self.assertEqual(len(cert), 0)



    def test_5_fork_at_did(self):
        #
        did_symbol = "test_fork_registerdiid"+common.get_random_str()
        rmtName = Zac.name+common.get_random_str()
        print( rmtName )
        mvs_rpc.new_address(Zac.name,Zac.password, 2)
        mvs_rpc.remote_call(self.remote_ip, mvs_rpc.import_account)(rmtName, "123456", ' '.join(Zac.mnemonic),2)
        receivers={}
        receivers[Zac.addresslist[0]] = (9**10)
        receivers[Zac.addresslist[1]] = (9**10)
        Alice.sendmore_etp(receivers)
        Alice.mining()

        # asset
        domain_symbol = ("Zacfork" + common.get_random_str()).upper()
        asset_symbol = domain_symbol + ".AST"

        # mit
        mit_symbol = ("MIT." + common.get_random_str()).upper()

        ec, message = mvs_rpc.get_info()
        self.assertEqual(ec, 0, message)
        pre_height = message[0]
        print( "pre_height:"+str(pre_height) )

        self.make_partion()
        import time
        try:
            # fork chain
            Alice.mining()
            ec, message = Zac.register_did(Zac.addresslist[0], did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            ec, message = mvs_rpc.change_did(Zac.name, Zac.password, Zac.addresslist[1], did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            Zac.create_asset_with_symbol(asset_symbol, True, 0, did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            Zac.send_asset(Zac.addresslist[1], 100 ,asset_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            mvs_rpc.transfer_cert(Zac.name, Zac.password, Alice.did_symbol, domain_symbol, "DOMAIN")
            self.assertEqual(ec, 0, message)
            Alice.mining()

            mvs_rpc.transfer_cert(Alice.name, Alice.password, "BLACKHOLE", domain_symbol, "DOMAIN")
            self.assertEqual(ec, 0, message)
            Alice.mining()

            ec, message = mvs_rpc.register_mit(Zac.name, Zac.password, did_symbol, mit_symbol, "test fork mit")
            self.assertEqual(ec, code.success, message)
            Alice.mining(5)

            ec, message = mvs_rpc.transfer_mit(Zac.name, Zac.password, "BLACKHOLE", mit_symbol)
            self.assertEqual(ec, code.success, message)
            Alice.mining(5)

        finally:
            # main chain
            self.remote_ming(2)
            ec, message = mvs_rpc.remote_call(self.remote_ip,mvs_rpc.register_did)(rmtName, "123456", Zac.addresslist[1],did_symbol)
            self.assertEqual(ec, 0, message)
            self.remote_ming(1)

            ec, message = mvs_rpc.remote_call(self.remote_ip,mvs_rpc.change_did)(rmtName, "123456", Zac.addresslist[0], did_symbol)
            self.assertEqual(ec, 0, message)
            self.remote_ming(1)

            ec, message = self.create_asset_with_symbol_rmt(rmtName, "123456", asset_symbol, True, 0, did_symbol)
            self.assertEqual(ec, 0, message)
            self.remote_ming(1)

            ec, message = mvs_rpc.remote_call(self.remote_ip, mvs_rpc.send_asset)(rmtName, "123456",Zac.addresslist[1], asset_symbol,100)
            self.assertEqual(ec, 0, message)
            self.remote_ming(1)

            ec, message = mvs_rpc.remote_call(self.remote_ip, mvs_rpc.transfer_cert)(rmtName, "123456", "BLACKHOLE", domain_symbol, "DOMAIN")
            self.assertEqual(ec, 0, message)
            self.remote_ming(1)

            ec, message = mvs_rpc.remote_call(self.remote_ip, mvs_rpc.register_mit)(rmtName, "123456", did_symbol, mit_symbol)
            self.assertEqual(ec, code.success, message)
            self.remote_ming(1)

            ec, message = mvs_rpc.remote_call(self.remote_ip,mvs_rpc.transfer_mit)(rmtName, "123456", "BLACKHOLE", mit_symbol)
            self.assertEqual(ec, code.success, message)
            self.remote_ming(20)






        ec, message = mvs_rpc.add_node( self.remote_ip+':5251')
        self.assertEqual(ec, 0, message)

        ec, message = mvs_rpc.remote_call(self.remote_ip, mvs_rpc.get_info)()
        self.assertEqual(ec, 0, message)
        main_height = message[0]

        ec, message = mvs_rpc.get_info()
        self.assertEqual(ec, 0, message)
        fork_height = message[0]
        while  fork_height < main_height:
            time.sleep(1)
            ec, message = mvs_rpc.get_info()
            self.assertEqual(ec, 0, message)
            fork_height = message[0]
            print( "fork_height:"+str(fork_height) + ",main_height:"+str(main_height) )

        ec, message = mvs_rpc.list_didaddresses(did_symbol)
        self.assertEqual(ec, 0, message)
        self.assertEqual(message[0]["address"], Zac.addresslist[0], message)


    def create_asset_with_symbol_rmt(self, name, password, symbol, is_issue, secondary, did_symbol):
        result, message = mvs_rpc.remote_call(self.remote_ip, mvs_rpc.create_asset)(name, password, symbol, 300000, did_symbol, description="%s's Asset" % name, rate=secondary)
        if (result):
            print("#ERROR#: failed to create asset: {}".format(message))
        assert (result == 0)
        if is_issue:
            result, message = mvs_rpc.remote_call(self.remote_ip,mvs_rpc.issue_asset)(name, password, symbol)
            if (result):
                print("#ERROR#: failed to issue asset: {}".format(message))
            assert (result == 0)
        return result, message
