{ nixpkgs ? import <nixpkgs> {} }:
let
  inherit (nixpkgs) pkgs;

  # esp-idf as of Nov 2019 requires pyparsing < 2.4
  python2 = let
    packageOverrides = self: super: {
            pyparsing = super.pyparsing.overridePythonAttrs( old: rec {
                version = "2.3.1";
                src = super.fetchPypi {
                    pname="pyparsing";
                    inherit version;
                    sha256="66c9268862641abcac4a96ba74506e594c884e3f57690a696d21ad8210ed667a";
                };
        });
    };
    in pkgs.python2.override{inherit packageOverrides; self= python2;};

  esp-idf = pkgs.stdenvNoCC.mkDerivation {
    name = "esp-idf-source";
    version = "3.3.1";

    buildCommand = ''git clone -b v3.3.1 --recursive https://github.com/espressif/esp-idf.git $out'';
    passAsFile = [ "buildCommand" ];

    nativeBuildInputs = [ pkgs.git ];
    GIT_SSL_CAINFO = "${pkgs.cacert}/etc/ssl/certs/ca-bundle.crt";
  };
in

pkgs.stdenv.mkDerivation {
  name = "esp-idf-env";

  src = ./src;

  dontConfigure = true;
  dontInstall = true;

  buildInputs = with pkgs; [
    gawk gperf gettext automake bison flex texinfo help2man libtool autoconf ncurses5 cmake glibcLocales
    (python2.withPackages (ppkgs: with ppkgs; [ pyserial future cryptography setuptools pyelftools pyparsing click ]))
    (pkgs.callPackage ./esp32-toolchain.nix {})
    git which
  ];
  GIT_SSL_CAINFO = "${pkgs.cacert}/etc/ssl/certs/ca-bundle.crt";

  IDF = esp-idf;

  buildPhase = ''
    cp -r $IDF esp-idf
    chmod -R +w esp-idf
    export IDF_PATH=$(pwd)/esp-idf
    export NIX_CFLAGS_LINK=-lncurses
    make

    mkdir $out
  '';

  shellHook = ''
    export IDF_TOOLS_PATH=$src/tools
    export PATH="$IDF_TOOLS_PATH:$PATH"
  '';
}