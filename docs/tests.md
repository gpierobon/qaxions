---
layout: default
title: Building & Testing
nav_order: 3
---

# Building 

Once all the dependencies are installed and ready to be found, to build the code
simply run from the root directory:

```
make
```

In order to disable the python tests and pybind, run:

```
make WITH_PYTHON=0
```


# Testing

```bash
pytest -v tests/
```
