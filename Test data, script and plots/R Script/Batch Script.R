plotter <- function(gid,rt){
par(mfrow=c(3,1),mar=c(2,8,4,2)+0.1)
ngest <- 8

gxlab <- "g"
gylab <- "gy"
wlav <- "w"
pAlab <- "pA"
pHlab <- "pH"
suf <- ".txt"
gx <- vector("list",ngest)
gy <- vector("list",ngest)
w  <- vector("list",ngest)
pA <- vector("list",ngest)
pHA <- vector("list",ngest);

for(i in 1:ngest){
    filen <- paste(gxlab,as.character(i),sep="")
    filen <- paste(filen,suf,sep="")
    gx[[i]] <- scan(filen);
    filen <- paste(gylab,as.character(i),sep="")
    filen <- paste(filen,suf,sep="")
    gy[[i]] <- scan(filen);
    filen <- paste(wlav,as.character(i),sep="")
    filen <- paste(filen,suf,sep="")
    w[[i]] <- scan(filen)
    filen <- paste(pAlab,as.character(i),sep="")
    filen <- paste(filen,suf,sep="")
    pA[[i]] <- scan(filen)
    filen <- paste(pHlab,as.character(i),sep="")
    filen <- paste(filen,suf,sep="")
    pHA[[i]] <- scan(filen)
}

 curGest <- scan("gest.txt")
 p1 <- scan("p1.txt")
 p2 <- scan("p2.txt")
 p3 <- scan("p3.txt")
 tot <- scan("tot.txt")
 ug <- scan("ug.txt")
 ugy<- scan("ugy.txt")
 rs <- scan("rs.txt")
 phase<-scan("phase.txt")
 phase2<-scan("phase2.txt")
 phase3<-scan("phase3.txt")
testVal <- scan("test.txt")
    
switcher<-scan("switcher.txt");
 
 v <- max(sapply(c(gx,gy,ug,ugy),max))
 vm <- min(sapply(c(gx,gy,ug,ugy),min))
 cols <- c("green","red","pink","purple","Gold","darkgrey","darkseagreen","brown")

#title1 <- sprintf("RT = %d Input Gesture = %s",rt,gid)
title1 <- sprintf("STD = %f Input Gesture = %s Score = %f",rt,gid,testVal)

 plot(0,type="l",ylim=c(vm,v),xlim=c(0,length(tot)),main=title1)
    # legend("bottomright", c("t1","t2","t3","user"), cex=0.8, col=c("red","pink","green","blue"),
    #lty=1:3, lwd=2, bty="n");
    
    for(i in 1:ngest){
        lines(gx[[i]],lty=1,col=cols[i])
        lines(gy[[i]],lty=1,col=cols[i])
    }
   lines(ug,lty=3,col="blue")
   lines(ugy,lty=3,col="blue")

    plot(0,type="l",ylim=c(-0.2,1),xlim=c(0,length(tot)),main="indv Prob",col="black",ylab="")
    # legend("topright", c("t1","t2","t3","total"), cex=0.8, col=c("red","pink","green","black"), lty=1:3, lwd=2, bty="n");
 #max(w[[1]],w[[2]],w[[3]],w[[4]],w[[5]],w[[6]],w[[7]],w[[8]])

 for(i in 1:ngest){
     lines(w[[i]],lty=1,col=cols[i])
     #lines(w[[i]],lty=1,col=cols[i])

    }
    #  plot(0,type="l",ylim=c(0,ngest),xlim=c(0,length(tot)),main="closest template",col="black",ylab="")

    #lines(curGest,lty=1,col="black");
    
    

 plot(0,type="l",ylim=c(-0.2,1),xlim=c(0,length(tot)),main="phase")
   points((switcher/9)*1-0.3,lty=5,col="orange",lwd=1,pch=20)   
    points((rs/9)*1-0.1,lty=3,col="blue",lwd=1,pch=20)
    
    # lines(phase,lty=5,col="green")

    
    for(i in 1:ngest){
        #lines(pA[[i]]/2000,lty=1,col=cols[i])
        lines(pHA[[i]],lty=1,col=cols[i])
    }
    
}


writeTest <- function(num){
    dag <-  "/Users/thomasrushmore/Desktop/data/run"
    x <- 0
    for(i in 1:6){
        x[i] <- 0
    }
    
    for(i in 1:num){
        for(j in 1:6){
            
            workdir <- paste(dag,as.character(((i-1)*6)+j),sep="")
            workdir <- paste(workdir,"/",sep="")
            print(workdir)
            setwd(workdir)
            val <- scan("test.txt")
            print(val)
            x[j] <- x[j] + val
            
        }
    }
    
    for(i in 1:6){
        x[i] <- x[i] / num
    }
    print(x)
}




batchplotter <- function(num){
dag <-  "/Users/thomasrushmore/Desktop/data/run"
pics <- "/Users/thomasrushmore/Desktop/data/screenshots/"
#rta <- c(0,100,250,500,750,1000)
rta <- c(0.1000000, 0.1166667, 0.1333333, 0.1500000, 0.1666667, 0.1833333)
for(i in 1:num){
for(j in 1:6){

workdir <- paste(dag,as.character(((i-1)*6)+j),sep="")
workdir <- paste(workdir,"/",sep="")
print(i)
print(workdir)
setwd(workdir)
photdir <- paste(pics,as.character(((i-1)*6)+j),sep="")
photodir <- paste(photdir,".pdf",sep="")
pdf(photodir)
plotter(i,rta[j]);
dev.off()

}
}
}

batchplotter(14)
writeTest(14) 