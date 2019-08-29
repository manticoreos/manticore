.. Top-Level documentation file is used to start the toctree; 
   however, this file is overriden in the rendering process
   by an alternative index.html that provides a visually-pleasing 
   landing page.  Updates to this top-level item must also
   be transposed in the 'templates/index.html' file.

Manticore Documentation
------------------------

.. toctree::
   :maxdepth: 1
   :glob:

   User's Guide <guides/user>
   Architecture Guide <guides/architecture>
   Contributor's Guide <guides/contributors>
   Research Guide <guides/research>
   Installation Guide <guides/installation>
   Doxygen Documentation <inline_docs/doxygen_docs>
