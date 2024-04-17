ARG FROM_IMAGE=ubuntu:22.04
FROM ${FROM_IMAGE}

RUN apt update \
 && apt install -y --no-install-recommends \
     ca-certificates \
     git \
     make \
     patch \
     asciidoc \
     latex-cjk-all \
     latexdiff \
     latexmk \
     lmodern \
     texlive \
     texlive-fonts-extra \
     texlive-lang-chinese \
     texlive-latex-extra \
     texlive-plain-generic \
 && rm -rf /var/lib/apt/lists/*

