plotter <- function(gid,rt){
par(mfrow=c(3,1),mar=c(2,8,4,2)+0.1)
ngest <- 8

gxlab <- "g"
gylab <- "gy"
wlav <- "w"
suf <- ".txt"
gx <- vector("list",ngest)
gy <- vector("list",ngest)
w  <- vector("list",ngest)

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
}

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
 
 v <- max(sapply(c(gx,gy,ug,ugy),max))
 vm <- min(sapply(c(gx,gy,ug,ugy),min))
 cols <- c("green","red","pink","orange","darkred","deepskyblue","red","yellow","violetred2")

 title1 <- sprintf("RT = %d Input Gesture = %s",rt,gid)
 
 plot(0,type="l",ylim=c(vm,v),xlim=c(0,length(tot)),main=title1)
 legend("bottomright", c("t1","t2","t3","user"), cex=0.8, col=c("red","pink","green","blue"), 
    lty=1:3, lwd=2, bty="n");
    
    for(i in 1:ngest){
        lines(gx[[i]],lty=1,col=cols[i])
        lines(gy[[i]],lty=1,col=cols[i])
    }
   lines(ug,lty=3,col="blue")
   lines(ugy,lty=3,col="blue")

 plot(tot/2000,type="l",ylim=c(-0.2,1),xlim=c(0,length(tot)),main="indv Prob",col="black",ylab="")
 legend("topright", c("t1","t2","t3","total"), cex=0.8, col=c("red","pink","green","black"), lty=1:3, lwd=2, bty="n");  

 for(i in 1:ngest){
        lines(w[[i]],lty=1,col=cols[i])
    }

 plot(0,type="l",ylim=c(-0.2,1),xlim=c(0,length(tot)),main="phase")
   points((rs/9)*1-0.3,lty=5,col="orange",lwd=1,pch=20)>   lines(phase,lty=5,col="green")
   lines(phase2,lty=5,col="red")
   lines(phase3,lty=5,col="pink")
}

batchplotter <- function(num){
dag <-  "/Users/thomasrushmore/Desktop/data/run"
pics <- "~/Desktop/data/screenshots/"
rta <- c(0,10,20)
for(i in 1:num){
for(j in 1:3){

workdir <- paste(dag,as.character(((i-1)*3)+j),sep="")
workdir <- paste(workdir,"/",sep="")
print(i)
print(workdir)
setwd(workdir)
photdir <- paste(pics,as.character(((i-1)*3)+j),sep="")
photodir <- paste(photdir,".pdf",sep="")
pdf(photodir)
plotter(i,rta[j]);
dev.off()

}
}
}

batchplotter(8)