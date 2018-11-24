import h5py
import numpy as np

file = h5py.File('archive.h5', 'w')

file['/scalar/double'] = 12.345
file['/scalar/int'   ] = 12345
file['/scalar/string'] = "12345"

file['/vector/double'] = np.array([1.1, 2.2, 3.3, 4.4, 5.5], dtype='float64')
file['/vector/int'   ] = np.array([1  , 2  , 3  , 4  , 5  ], dtype='int')

file['matrix/double' ] = np.array([[1.1, 2.2], [3.3, 4.4], [5.5, 6.6]], dtype='float64')
file['matrix/int'    ] = np.array([[1  , 2  ], [3  , 4  ], [5  , 6  ]], dtype='int')

file.close()

