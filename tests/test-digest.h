/* Test of message digests.
   Copyright (C) 2018 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

static void
test_digest_on_files (int (*streamfunc) (FILE *, void *),
                      const char *streamfunc_name,
                      size_t digest_size,
                      const void *expected_for_empty_file,
                      const void *expected_for_small_file,
                      const void *expected_for_large_file)
{
  int pass;
  unlink (TESTFILE);

  for (pass = 0; pass < 3; pass++)
    {
      {
        FILE *fp = fopen (TESTFILE, "wb");
        if (fp == NULL)
          {
            fprintf (stderr, "Could not create file %s.\n", TESTFILE);
            exit (1);
          }
        switch (pass)
          {
          case 0:
            /* Nothing to do for the empty file.  */
            break;
          case 1:
            /* Fill the small file.  */
            fputs ("The quick brown fox jumps over the lazy dog.\n", fp);
            break;
          case 2:
            /* Fill the large file (8 MiB).  */
            {
              unsigned int i;
              for (i = 0; i < 0x400000; i++)
                {
                  unsigned char c[2];
                  unsigned int j = i * (i-1) * (i-5);
                  c[0] = (unsigned char)(j >> 6);
                  c[1] = (i % 499) + (i % 101);
                  fwrite (c, 1, 2, fp);
                }
            }
            break;
          }
        if (ferror (fp))
          {
            fprintf (stderr, "Could not write data to file %s.\n", TESTFILE);
            exit (1);
          }
        fclose (fp);
      }
      {
        /* Test an unaligned digest.  */
        char *digest = (char *) malloc (digest_size + 1) + 1;
        const void *expected;
        FILE *fp;
        int ret;

        switch (pass)
          {
          case 0: expected = expected_for_empty_file; break;
          case 1: expected = expected_for_small_file; break;
          case 2: expected = expected_for_large_file; break;
          default: abort ();
          }

        fp = fopen (TESTFILE, "rb");
        if (fp == NULL)
          {
            fprintf (stderr, "Could not open file %s.\n", TESTFILE);
            exit (1);
          }
        ret = streamfunc (fp, digest);
        if (ret)
          {
            fprintf (stderr, "%s failed with error %d\n", streamfunc_name, -ret);
            exit (1);
          }
        if (memcmp (digest, expected, digest_size) != 0)
          {
            int i;
            fprintf (stderr, "%s produced wrong result.\n", streamfunc_name);
            fprintf (stderr, "Expected: ");
            for (i = 0; i < digest_size; i++)
              fprintf (stderr, "\\x%02x", ((const unsigned char *) expected)[i]);
            fprintf (stderr, "\n");
            fprintf (stderr, "Got:      ");
            for (i = 0; i < digest_size; i++)
              fprintf (stderr, "\\x%02x", ((const unsigned char *) digest)[i]);
            fprintf (stderr, "\n");
            exit (1);
          }
        fclose (fp);
        free (digest - 1);
      }
    }

  unlink (TESTFILE);
}