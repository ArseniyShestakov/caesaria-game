// This file is part of CaesarIA.
//
// CaesarIA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CaesarIA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CaesarIA.  If not, see <http://www.gnu.org/licenses/>.

#include "core/exception.hpp"
#include "filepath.hpp"
#include "filesystem.hpp"
#include "filelist.hpp"
#include "directory.hpp"

#ifdef CAESARIA_PLATFORM_WIN
  #include <windows.h>
  #include <io.h>
#elif defined(CAESARIA_PLATFORM_UNIX)
  #ifdef CAESARIA_PLATFORM_LINUX
    #include <sys/io.h>
    #include <linux/limits.h>
  #elif defined(CAESARIA_PLATFORM_MACOSX)
    #include <libproc.h>
  #endif
  #include <sys/stat.h>
  #include <unistd.h>
  #include <stdio.h>
  #include <libgen.h>
#endif

namespace vfs
{

const char* Path::anyFile = "*.*";
const char* Path::firstEntry = ".";
const char* Path::secondEntry = "..";

class Path::Impl
{
public:
  std::string path;
};

void Path::splitToDirPathExt( Path* path,
                      Path* filename,
                      Path* extension )
{
  std::string name = _d->path;;
  int i = name.size();
  int extpos = i;

  // search for path separator or beginning
  while ( i >= 0 )
  {
      if ( name[i] == '.' )
      {
          extpos = i;
          if( extension )
          {
              *extension = Path( name.substr( extpos + 1, name.size() - (extpos + 1) ) );
          }
      }
      else
          if ( name[i] == '/' || name[i] == '\\' )
          {
              if ( filename )
              {
                  *filename = Path( name.substr( i + 1, extpos - (i + 1) ) );
              }

              if ( path )
              {
                std::string rp = StringHelper::replace( name.substr( 0, i + 1 ), "\\", "/" ) ;   
                *path = Path( rp );
              }
              return;
          }
      i -= 1;
  }

  if ( filename )
  {
      *filename = Path( name.substr( 0, extpos ) );
  }
}

Path Path::addEndSlash() const
{
  std::string pathTo = _d->path;

  if( pathTo.size() == 0 )
      return Path( "" );

  if( (*pathTo.rbegin()) != '/' && (*pathTo.rbegin()) != '\\' )
  {
      pathTo.append( "/" );
  }

  return pathTo;
}

Path Path::removeBeginSlash() const
{
  std::string pathTo = _d->path;
 
  if( pathTo.empty() )
      return Path( "" ); 

  wchar_t endsym = *pathTo.begin();
  if( endsym == '/' || endsym == '\\' )
      pathTo.erase( 0 );

  return pathTo;
}

Path Path::removeEndSlash() const
{
  std::string pathTo = _d->path;
    
  if( pathTo.empty() )
      return "";

  if( (*pathTo.rbegin()) == '/' || (*pathTo.rbegin()) == '\\' )
  {
      pathTo.erase( pathTo.rbegin().base() );
  }

  return pathTo;
}

bool Path::isExist() const
{
    return FileSystem::instance().existFile( *this );
}

bool Path::isFolder() const
{
#ifdef CAESARIA_PLATFORM_WIN
  WIN32_FILE_ATTRIBUTE_DATA fad;
  if( ::GetFileAttributesEx( _d->path.c_str(), ::GetFileExInfoStandard, &fad )== 0 )
      return false;

  return (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
#elif defined(CAESARIA_PLATFORM_UNIX)
  struct stat path_stat;
  if ( ::stat( toString().c_str(), &path_stat) != 0 )
      return false;

  return S_ISDIR(path_stat.st_mode);
#endif //CAESARIA_PLATFORM_UNIX
}

std::string Path::getExtension() const 
{
    if( isFolder() )
    {
        return "";
    }

    std::string::size_type index = _d->path.find_last_of( '.' );
    if( index != std::string::npos )
    {
        return _d->path.substr( index, 0xff );
    }

    return "";
}

Path::Path( const std::string& nPath ) : _d( new Impl )
{
  _d->path = StringHelper::replace( nPath, "\\", "/" );
}

Path::Path( const Path& nPath ) : _d( new Impl )
{
  _d->path = nPath._d->path;
}

Path::Path( const char* nPath ) : _d( new Impl )
{
  _d->path = StringHelper::replace( nPath, "\\", "/" );
}

Path::Path() : _d( new Impl )
{
  _d->path = "";
}

const std::string& Path::toString() const
{
  return _d->path;
}

std::string Path::removeExtension() const
{
  std::string::size_type index = _d->path.find_last_of( '.' );
  if( index != std::string::npos )
  {
    return _d->path.substr( 0, index );
  }

  return toString();
}

Path::~Path()
{

}

Path Path::getAbsolutePath() const
{
#if defined(CAESARIA_PLATFORM_WIN)
  char *p=0;
  char fpath[_MAX_PATH];

  p = _fullpath(fpath, _d->path.c_str(), _MAX_PATH);
  std::string tmp = StringHelper::replace( p, "\\", "/");
  return tmp;
#elif defined(CAESARIA_PLATFORM_UNIX)
  char* p=0;
  char fpath[4096];
  fpath[0]=0;
  p = realpath( _d->path.c_str(), fpath);
  if (!p)
  {
    // content in fpath is unclear at this point
    if (!fpath[0]) // seems like fpath wasn't altered, use our best guess
    {
      FilePath tmp(_d->path);
      return flattenFilename(tmp);
    }
    else
      return FilePath(fpath);
  }

  if( *(_d->path.rbegin())=='/')
    return FilePath( std::string(p) + "/" );
  else
    return FilePath( std::string(p) );
#endif // CAESARIA_PLATFORM_UNIX
}


//! flatten a path and file name for example: "/you/me/../." becomes "/you"
Path Path::flattenFilename( const Path& root ) const
{
  std::string directory = addEndSlash().toString();
  directory = StringHelper::replace( directory, "\\", "/" );
  
  Directory dir;
  Path subdir;

  int lastpos = 0;
  std::string::size_type pos = 0;
  bool lastWasRealDir=false;

  while( ( pos = directory.find( '/', lastpos) ) != std::string::npos )
  {
    subdir = Path( directory.substr(lastpos, pos - lastpos + 1) );

    if( subdir.toString() == "../" )
    {
      if (lastWasRealDir)
      {
        dir = dir.up().up();
        lastWasRealDir=( dir.toString().size()!=0);
      }
      else
      {
        dir = Path( dir.toString() + subdir.toString() );
        lastWasRealDir=false;
      }
    }
    else if( subdir.toString() == "/")
    {
      dir = root;
    }
    else if( subdir.toString() != "./" )
    {
      dir = Path( dir.toString() + subdir.toString() );
      lastWasRealDir=true;
    }

    lastpos = pos + 1;
  }

  return dir;
}

//! Get the relative filename, relative to the given directory
Path Path::getRelativePathTo( const Path& directory ) const
{
  if ( toString().empty() || directory.toString().empty() )
    return *this;

  Path path1, file, ext;
  getAbsolutePath().splitToDirPathExt( &path1, &file, &ext );
  Path path2(directory.getAbsolutePath());
  StringArray list1, list2;

  list1 = StringHelper::split( path1.toString(), "/\\", 2);
  list2 = StringHelper::split( path2.toString(), "/\\", 2);

  unsigned int i=0;
  unsigned int it1=0;
  unsigned int it2=0;

#if defined (CAESARIA_PLATFORM_WIN)
  char partition1 = 0, partition2 = 0;
  Path prefix1, prefix2;
  if ( it1 > 0 )
    prefix1 = list1[ it1 ];
  if ( it2 > 0 )
    prefix2 = list2[ it2 ];

  if ( prefix1.toString().size() > 1 && prefix1.toString()[1] == ':' )
  {
    partition1 = StringHelper::localeLower( prefix1.toString()[0] );
  }

  if ( prefix2.toString().size() > 1 && prefix2.toString()[1] == ':' )
  {
    partition2 = StringHelper::localeLower( prefix2.toString()[0] );
  }

  // must have the same prefix or we can't resolve it to a relative filename
  if ( partition1 != partition2 )
  {
    return *this;
  }
#endif //CAESARIA_PLATFORM_WIN


  for (; i<list1.size() && i<list2.size() 
#if defined (CAESARIA_PLATFORM_WIN)
    && ( StringHelper::isEquale( list1[ it1 ], list2[ it2 ], StringHelper::equaleIgnoreCase ) )
#elif defined(CAESARIA_PLATFORM_UNIX)
    && ( list1[ it1 ]== list2[ it2 ] )	
#endif //CAESARIA_PLATFORM_UNIX
    ; ++i)
  {
    ++it1;
    ++it2;
  }

  path1="";
  for (; i<list2.size(); ++i)
  {
    path1 = path1.toString() + "../";
  }

  while( it1 != list1.size() )
  {
    path1 = path1.toString() + list1[ it1 ] + "/";
    it1++;
  }

  path1 = path1.toString() + file.toString();
  if( ext.toString().size() )
  {
    path1 = path1.toString() + "." + ext.toString();
  }
  return path1;
}

//! returns the base part of a filename, i.e. all except for the directory
//! part. If no directory path is prefixed, the full name is returned.
Path Path::getBasename(bool keepExtension) const
{
  // find last forward or backslash
  std::string::size_type lastSlash = toString().find_last_of('/');

  // get number of chars after last dot
  std::string::size_type end = 0;
  if( !keepExtension )
  {
    // take care to search only after last slash to check only for
    // dots in the filename
    end = toString().find_last_of('.');
    if( end == std::string::npos || end < lastSlash)
      end=0;
    else
      end = toString().size()-end;
  }

  if( lastSlash != std::string::npos )
  {
    return Path( toString().substr(lastSlash+1, toString().size()-lastSlash-1-end) );
  }
  else if (end != 0)
  {
    return Path( toString().substr(0, toString().size()-end) );
  }
  else
  {
    return *this;
  }
}

//! returns the directory part of a filename, i.e. all until the first
//! slash or backslash, excluding it. If no directory path is prefixed, a '.'
//! is returned.
std::string Path::getDir() const
{
  // find last forward or backslash
  std::string::size_type lastSlash = toString().find_last_of( '/' );

  if( lastSlash != std::string::npos )
  {
    return toString().substr(0, lastSlash);
  }
  else
    return ".";
}

bool Path::operator==( const Path& other ) const
{
  return toString() == other.toString();
}

bool Path::operator==( const std::string& other ) const
{
    return toString() == other;
}

char &Path::operator [](const unsigned int index)
{
    return _d->path[index];
}

bool Path::isExtension(const std::string &ext, bool checkCase) const
{
    return StringHelper::isEquale( getExtension(), ext, checkCase ? StringHelper::equaleCase : StringHelper::equaleIgnoreCase );
}

Path& Path::operator=( const Path& other )
{
  _d->path = other._d->path;
  return *this;
}

Path &Path::operator +=(char c)
{
  _d->path += c;
  return *this;
}

} //end namespace vfs
