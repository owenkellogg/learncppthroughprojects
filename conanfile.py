from conans import ConanFile

class ConanPackage(ConanFile):
    name = 'network-monitor'
    version = "0.1.0"

    generators = 'cmake_find_package'

    requires = [
        ('boost/1.82.0'),
        ('openssl/1.1.1w'),
    ]

    default_options = (
        'boost:shared=False',
        'openssl:shared=False',
    )

    def remotes(self, clean=False):
        return ["conancenter https://center2.conan.io"]