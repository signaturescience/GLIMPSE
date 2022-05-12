# GLIMPSE Tools

[![](docs/images/branding/glimpse_logo_250x107.png)](https://odelaneau.github.io/GLIMPSE/)

[![MIT License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

## Docker

### Usage

```sh
docker pull stephenturner/glimpse

docker run --rm stephenturner/glimpse chunk
docker run --rm stephenturner/glimpse concordance
docker run --rm stephenturner/glimpse ligate
docker run --rm stephenturner/glimpse phase
docker run --rm stephenturner/glimpse sample
docker run --rm stephenturner/glimpse snparray
docker run --rm stephenturner/glimpse stats
```

Alternatively, use singularity:

```sh
singularity pull glimpse.sif docker://stephenturner/glimpse

singularity run glimpse.sif chunk
singularity run glimpse.sif concordance
singularity run glimpse.sif ligate
singularity run glimpse.sif phase
singularity run glimpse.sif sample
singularity run glimpse.sif snparray
singularity run glimpse.sif stats
```

### Build

```sh
git pull https://github.com/stephenturner/GLIMPSE
cd GLIMPSE
docker build --no-cache -t glimpse .
```

## Versions

**Current Release: 1.1.1**. Release date: May 21, 2021

For details of past changes please see [CHANGELOG](versions/CHANGELOG.md).

## License

GLIMPSE is available under a MIT license. For more information please see the [LICENSE](LICENSE).
 
## Features

GLIMPSE is a set of tools for phasing and imputation for low-coverage sequencing datasets:

- **GLIMPSE_chunk** splits the genome into chunks ready for imputation/phasing
- **GLIMPSE_phase** phases & imputes low coverage sequencing data
- **GLIMPSE_ligate** concatenates phased chunks of data into chromosome wide files
- **GLIMPSE_sample** allows sampling plausible haplotype configurations from the files

## Documentation and tutorials

Please visit our website for tutorials, documentation and installation instructions:

https://odelaneau.github.io/GLIMPSE/
