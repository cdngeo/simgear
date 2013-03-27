/*
  Copied from PLIB into SimGear
  
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.
 
     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA
 
     For further information visit http://plib.sourceforge.net

     $Id: netChannel.cxx 1906 2004-03-22 19:44:50Z sjbaker $
*/

// TODO:
// have all socket-related functions assert that the socket has not
// been closed.  [a read event may close it, and a write event may try
// to write or something...]
// Maybe assert valid handle, too?

#include "sg_netChannel.hxx"

#include <memory>
#include <cassert>
#include <cstring>
#include <errno.h>

#include <simgear/debug/logstream.hxx>


namespace simgear  {

static NetChannel* channels = 0 ;

NetChannel::NetChannel ()
{
  closed = true ;
  connected = false ;
  resolving_host  = false;
  accepting = false ;
  write_blocked = false ;
  should_delete = false ;

  next_channel = channels ;
  channels = this ;
}
  
NetChannel::~NetChannel ()
{
  close();

  NetChannel* prev = NULL ;

  for ( NetChannel* ch = channels; ch != NULL;
                    ch = ch -> next_channel )
  {
    if (ch == this)
    {
      ch = ch -> next_channel ;
      if ( prev != NULL )
        prev -> next_channel = ch ;
      else
        channels = ch ;
      next_channel = 0 ;
      break;
    }
    prev = ch ;
  }
}
  
void
NetChannel::setHandle (int handle, bool is_connected)
{
  close () ;
  Socket::setHandle ( handle ) ;
  connected = is_connected ;
  closed = false ;
}

bool
NetChannel::open (void)
{
  close();
  if (Socket::open(true)) {
    closed = false ;
    setBlocking ( false ) ;
    return true ;
  }
  return false ;
}

int
NetChannel::listen ( int backlog )
{
  accepting = true ;
  return Socket::listen ( backlog ) ;
}

int
NetChannel::connect ( const char* h, int p )
{
  host = h;
  port = p;
  resolving_host = true;
  return handleResolve();
}

int
NetChannel::send (const void * buffer, int size, int flags)
{
  int result = Socket::send (buffer, size, flags);
  
  if (result == (int)size) {
    // everything was sent
    write_blocked = false ;
    return result;
  } else if (result >= 0) {
    // not all of it was sent, but no error
    write_blocked = true ;
    return result;
  } else if (isNonBlockingError ()) {
    write_blocked = true ;
    return 0;
  } else {
    this->handleError (result);
    close();
    return -1;
  }
  
}

int
NetChannel::recv (void * buffer, int size, int flags)
{
  int result = Socket::recv (buffer, size, flags);
  
  if (result > 0) {
    return result;
  } else if (result == 0) {
    close();
    return 0;
  } else if (isNonBlockingError ()) {
    return 0;
  } else {
    this->handleError (result);
    close();
    return -1;
  }
}

void
NetChannel::close (void)
{
  if ( !closed )
  {
    this->handleClose();
  
    closed = true ;
    connected = false ;
    accepting = false ;
    write_blocked = false ;
  }

  Socket::close () ;
}

void
NetChannel::handleReadEvent (void)
{
  if (accepting) {
    if (!connected) {
      connected = true ;
    }
    this->handleAccept();
  } else if (!connected) {
    connected = true ;
    this->handleRead();
  } else {
    this->handleRead();
  }
}

void
NetChannel::handleWriteEvent (void)
{
  if (!connected) {
    connected = true ;
  }
  write_blocked = false ;
  this->handleWrite();
}

int
NetChannel::handleResolve()
{
    IPAddress addr;
    if (!IPAddress::lookupNonblocking(host.c_str(), addr)) {
        return 0; // not looked up yet, wait longer
    }
    
    if (!addr.isValid()) {
        SG_LOG(SG_IO, SG_WARN, "Network: host lookup failed:" << host);
        handleError (ENOENT);
        close();
        return -1;
    }
    
    resolving_host = false;
    addr.setPort(port);
    int result = Socket::connect ( &addr ) ;
    if (result == 0) {
        connected = true ;
        return 0;
    } else if (isNonBlockingError ()) {
        return 0;
    } else {
        // some other error condition
        handleError (result);
        close();
        return -1;
    }
}

bool
NetChannel::poll (unsigned int timeout)
{
  if (!channels)
    return false ;
  
  enum { MAX_SOCKETS = 256 } ;
  Socket* reads [ MAX_SOCKETS+1 ] ;
  Socket* writes [ MAX_SOCKETS+1 ] ;
  Socket* deletes [ MAX_SOCKETS+1 ] ;
  int nreads = 0 ;
  int nwrites = 0 ;
  int ndeletes = 0 ;
  int nopen = 0 ;
  NetChannel* ch;
  for (  ch = channels; ch != NULL; ch = ch -> next_channel )
  {
    if ( ch -> should_delete )
    {
      assert(ndeletes<MAX_SOCKETS);
      deletes[ndeletes++] = ch ;
    }
    else if ( ! ch -> closed )
    {
      if (ch -> resolving_host )
      {
          ch -> handleResolve();
          continue;
      }
      
      nopen++ ;
      if (ch -> readable()) {
        assert(nreads<MAX_SOCKETS);
        reads[nreads++] = ch ;
      }
      if (ch -> writable()) {
        assert(nwrites<MAX_SOCKETS);
        writes[nwrites++] = ch ;
      }
    }
  }
  reads[nreads] = NULL ;
  writes[nwrites] = NULL ;
  deletes[ndeletes] = NULL ;

  int i ;
  for ( i=0; deletes[i]; i++ )
  {
    ch = (NetChannel*)deletes[i];
    delete ch ;
  }

  if (!nopen)
    return false ;
  if (!nreads && !nwrites)
    return true ; //hmmm- should we shutdown?

  Socket::select (reads, writes, timeout) ;

  for ( i=0; reads[i]; i++ )
  {
    ch = (NetChannel*)reads[i];
    if ( ! ch -> closed )
      ch -> handleReadEvent();
  }

  for ( i=0; writes[i]; i++ )
  {
    ch = (NetChannel*)writes[i];
    if ( ! ch -> closed )
      ch -> handleWriteEvent();
  }

  return true ;
}

void
NetChannel::loop (unsigned int timeout)
{
  while ( poll (timeout) ) ;
}


void NetChannel::handleRead (void) {
  SG_LOG(SG_IO, SG_WARN, "Network:" << getHandle() << ": unhandled read");
}

void NetChannel::handleWrite (void) {
  SG_LOG(SG_IO, SG_WARN, "Network:" << getHandle() << ": unhandled write");
}

void NetChannel::handleAccept (void) {
  SG_LOG(SG_IO, SG_WARN, "Network:" << getHandle() << ": unhandled accept");
}

void NetChannel::handleError (int error)
{
    // warn about address lookup failures seperately, don't warn again.
    // (and we (ab-)use ENOENT to mean 'name not found'.
    if (error != ENOENT) {
        SG_LOG(SG_IO, SG_WARN,"Network:" << getHandle() << ": errno: " << strerror(errno) <<"(" << errno << ")");
    }
}

} // of namespace simgear