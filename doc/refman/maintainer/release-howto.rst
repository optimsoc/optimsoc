OpTiMSoC Release HOWTO
======================

What needs to be done to release a new OpTiMSoC version?

OpTiMSoC releases are named YEAR.N, where YEAR is the current year
(four-digit), and N is a incrementing number starting at 1 representing
the Nth release in YEAR.

1. Tag the source code in git on the ``master`` branch

  We use annotated tags for releases to record the release author and the
  release date properly. All release version tags start with "v".
   
  .. code-block:: shell

    git tag -am "Release vYEAR.N" vYEAR.N

2. Build the code for this release

   .. code-block:: shell
   
     cd $OPTIMSOC_SRC
     make dist
     
   The resulting binary packages are named ``objdir/optimsoc-YEAR.N-[base|examples].tar.gz``

3. Test the release and make sure it works.

4. Push the tag to the remote repository
   
   .. code-block:: shell
   
     git push origin vYEAR.N

5. Follow the `GitHub documentation <https://help.github.com/articles/creating-releases/>`_
   on how to create a release.
   In this process you can also upload the created binary packages.
