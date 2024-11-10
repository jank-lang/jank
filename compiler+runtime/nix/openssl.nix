{
  pkgs,
}:

pkgs.stdenv.mkDerivation rec {
  pname = "openssl";
  version = "3.3.2";

  src = pkgs.fetchFromGitHub {
    owner = pname;
    repo = pname;
    rev = "${pname}-${version}";
    hash = "sha256-3KB0fetgXloCniFsvzzuchKgopPdQdh9/00M1mqJWyg=";
  };

  buildInputs = [
    pkgs.perl
  ];

  configurePhase = ''
    perl ./Configure --prefix=$out no-shared
  '';

  buildPhase = ''
    make
  '';

  installPhase = ''
    make install
  '';
}
