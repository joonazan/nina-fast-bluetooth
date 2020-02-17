{ stdenv, fetchurl, makeWrapper, buildFHSUserEnv }:

let
  fhsEnv = buildFHSUserEnv {
    name = "esp32-toolchain-env";
    targetPkgs = pkgs: with pkgs; [ zlib ];
    runScript = "";
  };
in

stdenv.mkDerivation rec {
  name = "esp32-toolchain";
  version = "2019r2";

  src = fetchurl {
    url = "https://dl.espressif.com/dl/xtensa-esp32-elf-gcc8_2_0-esp-2019r2-linux-amd64.tar.gz";
    sha256 = "1pzv1r9kzizh5gi3gsbs6jg8rs1yqnmf5rbifbivz34cplfprm76";
  };

  buildInputs = [ makeWrapper ];

  phases = [ "unpackPhase" "installPhase" ];

  installPhase = ''
    cp -r . $out
    for FILE in $(ls $out/bin); do
      FILE_PATH="$out/bin/$FILE"
      if [[ -x $FILE_PATH ]]; then
        mv $FILE_PATH $FILE_PATH-unwrapped
        makeWrapper ${fhsEnv}/bin/esp32-toolchain-env $FILE_PATH --add-flags "$FILE_PATH-unwrapped"
      fi
    done
  '';

  meta = with stdenv.lib; {
    description = "ESP32 toolchain";
    homepage = https://docs.espressif.com/projects/esp-idf/en/stable/get-started/linux-setup.html;
    license = licenses.gpl3;
  };
}