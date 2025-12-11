{
  description = "Python development environment with matplotlib";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };

        python = pkgs.python312; # or python311 if you prefer
        pythonPackages = python.pkgs;
      in {
        devShell = pkgs.mkShell {
          name = "python-matplotlib-dev";

          packages = with pythonPackages; [
            pip
            numpy
            pandas
            matplotlib
            pillow
            jupyter
            ipython
          ];

          # Useful for matplotlib backends & debugging
          buildInputs = [
            pkgs.pkg-config
          ];

          # Sometimes needed for GUI backends
          propagatedBuildInputs = [
            pkgs.libpng
            pkgs.freetype
          ];

          shellHook = ''
            echo "Python dev environment activated."
            echo "matplotlib version: $(python -c 'import matplotlib; print(matplotlib.__version__)')"
          '';
        };
      });
}
