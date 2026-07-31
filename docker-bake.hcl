variable "REPO" {
  default = "lifflander1/vt"
}

variable "GIT_BRANCH" {}

function "arch" {
  params = [item]
  result = lookup(item, "arch", "amd64")
}

function "comm_docs" {
  params = [item]
  result = lookup(item, "comm_docs", "0")
}

function "variant" {
  params = [item]
  result = lookup(item, "variant", "")
}

function "target_suffix" {
  params = [item]
  result = variant(item) == "" ? "" : "-${variant(item)}"
}

target "comm-build" {
  target = "build"
  context = "."
  dockerfile = "ci/docker/comm.dockerfile"

  platforms = [
    "linux/amd64",
    # "linux/arm64"
  ]
  ulimits = [
    "core=0"
  ]

  secret = ["id=GITHUB_TOKEN,env=GITHUB_TOKEN"]
}

target "comm-build-all" {
  name = "comm-build-${replace(item.image, ".", "-")}${target_suffix(item)}"
  inherits = ["comm-build"]
  tags = ["${REPO}:comm-${item.image}"]

  args = {
    ARCH = arch(item)
    GIT_BRANCH = "${GIT_BRANCH}"
    IMAGE = "wf-${item.image}"
    REPO = REPO
    COMM_DOXYGEN_ENABLED = comm_docs(item)
  }

  # to get the list of available images from DARMA-tasking/workflows:
  # workflows > docker buildx bake --print build-all | grep "lifflander1/vt:"
  matrix = {
    item = [
      {
        image = "amd64-alpine-3.16-clang-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-clang-12-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-clang-13-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-clang-14-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-clang-15-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-gcc-11-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-gcc-11-cpp"
        comm_docs = 1
        variant = "docs"
      },
      {
        image = "amd64-ubuntu-22.04-gcc-12-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-gcc-12-vtk-cpp"
      },
      {
        image = "amd64-ubuntu-22.04-gcc-12-zoltan-cpp"
      },
      {
        image = "amd64-ubuntu-24.04-clang-16-cpp"
      },
      {
        image = "amd64-ubuntu-24.04-clang-16-vtk-cpp"
      },
      {
        image = "amd64-ubuntu-24.04-clang-16-zoltan-cpp"
      },
      {
        image = "amd64-ubuntu-24.04-clang-17-cpp"
      },
      {
        image = "amd64-ubuntu-24.04-clang-18-cpp"
      },
      {
        image = "amd64-ubuntu-24.04-gcc-13-cpp"
      },
      {
        image = "amd64-ubuntu-24.04-gcc-14-cpp"
      }
    ]
  }
}
