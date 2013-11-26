// This file is part of openCaesar3.
//
// openCaesar3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openCaesar3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openCaesar3.  If not, see <http://www.gnu.org/licenses/>.


#ifndef __OPENCAESAR3_FILENATIVE_H_INCLUDED__
#define __OPENCAESAR3_FILENATIVE_H_INCLUDED__

#include <stdio.h>
#include "entity.hpp"
#include "filepath.hpp"

namespace vfs
{

class FileNative : public Entity
{
public:
	FileNative( const Path& fileName, Entity::Mode mode=Entity::fmRead );

	FileNative();

  virtual ~FileNative();

  //! returns how much was read
  virtual int read(void* buffer, unsigned int sizeToRead);

  virtual ByteArray readLine();

  virtual ByteArray read( unsigned int sizeToRead );

  virtual int write(const void* buffer, unsigned int sizeToWrite);

  virtual int write( const ByteArray& bArray );

  //! changes position in file, returns true if successful
  virtual bool seek(long finalPos, bool relativeMovement = false);

  //! returns size of file
  virtual long getSize() const;

  //! returns if file is open
  virtual bool isOpen() const;

  //! returns where in the file we are.
  virtual long getPos() const;

  //! returns name of file
  virtual const Path& getFileName() const;

  virtual bool isEof() const;

  virtual void flush();

private:

  //! opens the file
  void openFile();

  FILE* _file;
  long _size;
  Path _name;
	Entity::Mode _mode;
};

} //end namespace io

#endif
