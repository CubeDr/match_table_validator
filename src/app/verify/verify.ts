import { DoesntMeet, verifyMeets } from './meet';

export interface VerificationResult {
    doesntMeets: DoesntMeet[],
}

export function verify(table: string[][][]): VerificationResult {
    return {
        doesntMeets: verifyMeets(table),
    };
}